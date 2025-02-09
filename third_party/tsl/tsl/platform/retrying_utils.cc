/* Copyright 2016 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tsl/platform/retrying_utils.h"

#include "tsl/platform/env.h"
#include "tsl/platform/errors.h"
#include "tsl/platform/file_system.h"
#include "tsl/platform/random.h"

namespace tsl {

namespace {

bool IsRetriable(absl::StatusCode code) {
  switch (code) {
    case absl::StatusCode::kUnavailable:
    case absl::StatusCode::kDeadlineExceeded:
    case absl::StatusCode::kUnknown:
      return true;
    default:
      // OK also falls here.
      return false;
  }
}

}  // namespace

Status RetryingUtils::CallWithRetries(const std::function<Status()>& f,
                                      const RetryConfig& config) {
  return CallWithRetries(
      f,
      [](int64_t micros) {
        return Env::Default()->SleepForMicroseconds(micros);
      },
      config);
}

Status RetryingUtils::CallWithRetries(
    const std::function<Status()>& f,
    const std::function<void(int64_t)>& sleep_usec, const RetryConfig& config) {
  int retries = 0;
  while (true) {
    auto status = f();
    if (!IsRetriable(status.code())) {
      return status;
    }
    if (retries >= config.max_retries) {
      // Return AbortedError, so that it doesn't get retried again somewhere
      // at a higher level.
      return Status(
          absl::StatusCode::kAborted,
          strings::StrCat("All ", config.max_retries,
                          " retry attempts failed. The last failure: ",
                          status.error_message()));
    }
    int64_t delay_micros = 0;
    if (config.init_delay_time_us > 0) {
      const int64_t random_micros = random::New64() % 1000000;
      delay_micros = std::min(config.init_delay_time_us << retries,
                              config.max_delay_time_us) +
                     random_micros;
    }
    VLOG(1) << "The operation failed and will be automatically retried in "
            << (delay_micros / 1000000.0) << " seconds (attempt "
            << (retries + 1) << " out of " << config.max_retries
            << "), caused by: " << status.ToString();
    sleep_usec(delay_micros);
    retries++;
  }
}

Status RetryingUtils::DeleteWithRetries(
    const std::function<Status()>& delete_func, const RetryConfig& config) {
  bool is_retried = false;
  return RetryingUtils::CallWithRetries(
      [delete_func, &is_retried]() {
        const Status status = delete_func();
        if (is_retried && status.code() == error::NOT_FOUND) {
          return OkStatus();
        }
        is_retried = true;
        return status;
      },
      config);
}

}  // namespace tsl
