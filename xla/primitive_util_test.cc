/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

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

#include "xla/primitive_util.h"

#include <numeric>
#include <string>

#include "xla/status_macros.h"
#include "xla/test.h"
#include "xla/test_helpers.h"
#include "xla/types.h"
#include "xla/util.h"
#include "xla/xla_data.pb.h"

namespace xla {
namespace {

TEST(PrimitiveUtilTest, StringToPrimitiveType) {
  auto expect_ok_and_equal = [](const std::string& str,
                                PrimitiveType expected) {
    TF_ASSERT_OK_AND_ASSIGN(PrimitiveType actual,
                            primitive_util::StringToPrimitiveType(str));
    EXPECT_EQ(expected, actual);
  };
  expect_ok_and_equal("f32", F32);
  expect_ok_and_equal("tuple", TUPLE);
  expect_ok_and_equal("pred", PRED);
  expect_ok_and_equal("s32", S32);

  EXPECT_IS_NOT_OK(primitive_util::StringToPrimitiveType("F32").status());
  EXPECT_IS_NOT_OK(primitive_util::StringToPrimitiveType("Pred").status());
  EXPECT_IS_NOT_OK(primitive_util::StringToPrimitiveType("preD").status());
}

TEST(PrimitiveUtilTest, FloatTypes) {
  EXPECT_EQ(primitive_util::SignificandWidth(F32), 24);
  EXPECT_EQ(primitive_util::SignificandWidth(BF16), 8);
  EXPECT_EQ(primitive_util::ExponentWidth(F32), 8);
  EXPECT_EQ(primitive_util::ExponentWidth(BF16), 8);
}

TEST(PrimitiveUtilTest, CastPreservesValues) {
  bool expecteds[PrimitiveType_ARRAYSIZE][PrimitiveType_ARRAYSIZE];
  expecteds[PRED][PRED] = true;
  expecteds[PRED][S4] = true;
  expecteds[PRED][S8] = true;
  expecteds[PRED][S16] = true;
  expecteds[PRED][S32] = true;
  expecteds[PRED][S64] = true;
  expecteds[PRED][U4] = true;
  expecteds[PRED][U8] = true;
  expecteds[PRED][U16] = true;
  expecteds[PRED][U32] = true;
  expecteds[PRED][U64] = true;
  expecteds[PRED][F16] = true;
  expecteds[PRED][F32] = true;
  expecteds[PRED][F64] = true;
  expecteds[PRED][C64] = true;
  expecteds[PRED][BF16] = true;
  expecteds[PRED][C128] = true;
  expecteds[PRED][F8E5M2] = true;
  expecteds[PRED][F8E4M3FN] = true;
  expecteds[S4][PRED] = false;
  expecteds[S4][S4] = true;
  expecteds[S4][S8] = true;
  expecteds[S4][S16] = true;
  expecteds[S4][S32] = true;
  expecteds[S4][S64] = true;
  expecteds[S4][U4] = false;
  expecteds[S4][U8] = false;
  expecteds[S4][U16] = false;
  expecteds[S4][U32] = false;
  expecteds[S4][U64] = false;
  expecteds[S4][F16] = true;
  expecteds[S4][F32] = true;
  expecteds[S4][F64] = true;
  expecteds[S4][C64] = true;
  expecteds[S4][BF16] = true;
  expecteds[S4][C128] = true;
  expecteds[S4][F8E5M2] = true;
  expecteds[S4][F8E4M3FN] = true;
  expecteds[S8][PRED] = false;
  expecteds[S8][S4] = false;
  expecteds[S8][S8] = true;
  expecteds[S8][S16] = true;
  expecteds[S8][S32] = true;
  expecteds[S8][S64] = true;
  expecteds[S8][U4] = false;
  expecteds[S8][U8] = false;
  expecteds[S8][U16] = false;
  expecteds[S8][U32] = false;
  expecteds[S8][U64] = false;
  expecteds[S8][F16] = true;
  expecteds[S8][F32] = true;
  expecteds[S8][F64] = true;
  expecteds[S8][C64] = true;
  expecteds[S8][BF16] = true;
  expecteds[S8][C128] = true;
  expecteds[S8][F8E5M2] = false;
  expecteds[S8][F8E4M3FN] = false;
  expecteds[S16][PRED] = false;
  expecteds[S16][S4] = false;
  expecteds[S16][S8] = false;
  expecteds[S16][S16] = true;
  expecteds[S16][S32] = true;
  expecteds[S16][S64] = true;
  expecteds[S16][U4] = false;
  expecteds[S16][U8] = false;
  expecteds[S16][U16] = false;
  expecteds[S16][U32] = false;
  expecteds[S16][U64] = false;
  expecteds[S16][F16] = false;
  expecteds[S16][F32] = true;
  expecteds[S16][F64] = true;
  expecteds[S16][C64] = true;
  expecteds[S16][BF16] = false;
  expecteds[S16][C128] = true;
  expecteds[S16][F8E5M2] = false;
  expecteds[S16][F8E4M3FN] = false;
  expecteds[S32][PRED] = false;
  expecteds[S32][S4] = false;
  expecteds[S32][S8] = false;
  expecteds[S32][S16] = false;
  expecteds[S32][S32] = true;
  expecteds[S32][S64] = true;
  expecteds[S32][U4] = false;
  expecteds[S32][U8] = false;
  expecteds[S32][U16] = false;
  expecteds[S32][U32] = false;
  expecteds[S32][U64] = false;
  expecteds[S32][F16] = false;
  expecteds[S32][F32] = false;
  expecteds[S32][F64] = true;
  expecteds[S32][C64] = false;
  expecteds[S32][BF16] = false;
  expecteds[S32][C128] = true;
  expecteds[S32][F8E5M2] = false;
  expecteds[S32][F8E4M3FN] = false;
  expecteds[S64][PRED] = false;
  expecteds[S64][S4] = false;
  expecteds[S64][S8] = false;
  expecteds[S64][S16] = false;
  expecteds[S64][S32] = false;
  expecteds[S64][S64] = true;
  expecteds[S64][U4] = false;
  expecteds[S64][U8] = false;
  expecteds[S64][U16] = false;
  expecteds[S64][U32] = false;
  expecteds[S64][U64] = false;
  expecteds[S64][F16] = false;
  expecteds[S64][F32] = false;
  expecteds[S64][F64] = false;
  expecteds[S64][C64] = false;
  expecteds[S64][BF16] = false;
  expecteds[S64][C128] = false;
  expecteds[S64][F8E5M2] = false;
  expecteds[S64][F8E4M3FN] = false;
  expecteds[U4][PRED] = false;
  expecteds[U4][S4] = false;
  expecteds[U4][S8] = true;
  expecteds[U4][S16] = true;
  expecteds[U4][S32] = true;
  expecteds[U4][S64] = true;
  expecteds[U4][U4] = true;
  expecteds[U4][U8] = true;
  expecteds[U4][U16] = true;
  expecteds[U4][U32] = true;
  expecteds[U4][U64] = true;
  expecteds[U4][F16] = true;
  expecteds[U4][F32] = true;
  expecteds[U4][F64] = true;
  expecteds[U4][C64] = true;
  expecteds[U4][BF16] = true;
  expecteds[U4][C128] = true;
  expecteds[U4][BF16] = true;
  expecteds[U4][C128] = true;
  expecteds[U4][F8E5M2] = false;
  expecteds[U4][F8E4M3FN] = true;
  expecteds[U8][PRED] = false;
  expecteds[U8][S4] = false;
  expecteds[U8][S8] = false;
  expecteds[U8][S16] = true;
  expecteds[U8][S32] = true;
  expecteds[U8][S64] = true;
  expecteds[U8][U4] = false;
  expecteds[U8][U8] = true;
  expecteds[U8][U16] = true;
  expecteds[U8][U32] = true;
  expecteds[U8][U64] = true;
  expecteds[U8][F16] = true;
  expecteds[U8][F32] = true;
  expecteds[U8][F64] = true;
  expecteds[U8][C64] = true;
  expecteds[U8][BF16] = true;
  expecteds[U8][C128] = true;
  expecteds[U8][BF16] = true;
  expecteds[U8][C128] = true;
  expecteds[U8][F8E5M2] = false;
  expecteds[U8][F8E4M3FN] = false;
  expecteds[U16][PRED] = false;
  expecteds[U16][S4] = false;
  expecteds[U16][S8] = false;
  expecteds[U16][S16] = false;
  expecteds[U16][S32] = true;
  expecteds[U16][S64] = true;
  expecteds[U16][U4] = false;
  expecteds[U16][U8] = false;
  expecteds[U16][U16] = true;
  expecteds[U16][U32] = true;
  expecteds[U16][U64] = true;
  expecteds[U16][F16] = false;
  expecteds[U16][F32] = true;
  expecteds[U16][F64] = true;
  expecteds[U16][C64] = true;
  expecteds[U16][BF16] = false;
  expecteds[U16][C128] = true;
  expecteds[U16][F8E5M2] = false;
  expecteds[U16][F8E4M3FN] = false;
  expecteds[U32][PRED] = false;
  expecteds[U32][S4] = false;
  expecteds[U32][S8] = false;
  expecteds[U32][S16] = false;
  expecteds[U32][S32] = false;
  expecteds[U32][S64] = true;
  expecteds[U32][U4] = false;
  expecteds[U32][U8] = false;
  expecteds[U32][U16] = false;
  expecteds[U32][U32] = true;
  expecteds[U32][U64] = true;
  expecteds[U32][F16] = false;
  expecteds[U32][F32] = false;
  expecteds[U32][F64] = true;
  expecteds[U32][C64] = false;
  expecteds[U32][BF16] = false;
  expecteds[U32][C128] = true;
  expecteds[U32][F8E5M2] = false;
  expecteds[U32][F8E4M3FN] = false;
  expecteds[U64][PRED] = false;
  expecteds[U64][S4] = false;
  expecteds[U64][S8] = false;
  expecteds[U64][S16] = false;
  expecteds[U64][S32] = false;
  expecteds[U64][S64] = false;
  expecteds[U64][U4] = false;
  expecteds[U64][U8] = false;
  expecteds[U64][U16] = false;
  expecteds[U64][U32] = false;
  expecteds[U64][U64] = true;
  expecteds[U64][F16] = false;
  expecteds[U64][F32] = false;
  expecteds[U64][F64] = false;
  expecteds[U64][C64] = false;
  expecteds[U64][BF16] = false;
  expecteds[U64][C128] = false;
  expecteds[U64][F8E5M2] = false;
  expecteds[U64][F8E4M3FN] = false;
  expecteds[F16][PRED] = false;
  expecteds[F16][S4] = false;
  expecteds[F16][S8] = false;
  expecteds[F16][S16] = false;
  expecteds[F16][S32] = false;
  expecteds[F16][S64] = false;
  expecteds[F16][U4] = false;
  expecteds[F16][U8] = false;
  expecteds[F16][U16] = false;
  expecteds[F16][U32] = false;
  expecteds[F16][U64] = false;
  expecteds[F16][F16] = true;
  expecteds[F16][F32] = true;
  expecteds[F16][F64] = true;
  expecteds[F16][C64] = true;
  expecteds[F16][BF16] = false;
  expecteds[F16][C128] = true;
  expecteds[F16][F8E5M2] = false;
  expecteds[F16][F8E4M3FN] = false;
  expecteds[F32][PRED] = false;
  expecteds[F32][S4] = false;
  expecteds[F32][S8] = false;
  expecteds[F32][S16] = false;
  expecteds[F32][S32] = false;
  expecteds[F32][S64] = false;
  expecteds[F32][U4] = false;
  expecteds[F32][U8] = false;
  expecteds[F32][U16] = false;
  expecteds[F32][U32] = false;
  expecteds[F32][U64] = false;
  expecteds[F32][F16] = false;
  expecteds[F32][F32] = true;
  expecteds[F32][F64] = true;
  expecteds[F32][C64] = true;
  expecteds[F32][BF16] = false;
  expecteds[F32][C128] = true;
  expecteds[F32][F8E5M2] = false;
  expecteds[F32][F8E4M3FN] = false;
  expecteds[F64][PRED] = false;
  expecteds[F64][S4] = false;
  expecteds[F64][S8] = false;
  expecteds[F64][S16] = false;
  expecteds[F64][S32] = false;
  expecteds[F64][S64] = false;
  expecteds[F64][U4] = false;
  expecteds[F64][U8] = false;
  expecteds[F64][U16] = false;
  expecteds[F64][U32] = false;
  expecteds[F64][U64] = false;
  expecteds[F64][F16] = false;
  expecteds[F64][F32] = false;
  expecteds[F64][F64] = true;
  expecteds[F64][C64] = false;
  expecteds[F64][BF16] = false;
  expecteds[F64][C128] = true;
  expecteds[F64][F8E5M2] = false;
  expecteds[F64][F8E4M3FN] = false;
  expecteds[C64][PRED] = false;
  expecteds[C64][S4] = false;
  expecteds[C64][S8] = false;
  expecteds[C64][S16] = false;
  expecteds[C64][S32] = false;
  expecteds[C64][S64] = false;
  expecteds[C64][U4] = false;
  expecteds[C64][U8] = false;
  expecteds[C64][U16] = false;
  expecteds[C64][U32] = false;
  expecteds[C64][U64] = false;
  expecteds[C64][F16] = false;
  expecteds[C64][F32] = false;
  expecteds[C64][F64] = false;
  expecteds[C64][C64] = true;
  expecteds[C64][BF16] = false;
  expecteds[C64][C128] = true;
  expecteds[C64][F8E5M2] = false;
  expecteds[C64][F8E4M3FN] = false;
  expecteds[BF16][PRED] = false;
  expecteds[BF16][S4] = false;
  expecteds[BF16][S8] = false;
  expecteds[BF16][S16] = false;
  expecteds[BF16][S32] = false;
  expecteds[BF16][S64] = false;
  expecteds[BF16][U4] = false;
  expecteds[BF16][U8] = false;
  expecteds[BF16][U16] = false;
  expecteds[BF16][U32] = false;
  expecteds[BF16][U64] = false;
  expecteds[BF16][F16] = false;
  expecteds[BF16][F32] = true;
  expecteds[BF16][F64] = true;
  expecteds[BF16][C64] = true;
  expecteds[BF16][BF16] = true;
  expecteds[BF16][C128] = true;
  expecteds[BF16][F8E5M2] = false;
  expecteds[BF16][F8E4M3FN] = false;
  expecteds[C128][PRED] = false;
  expecteds[C128][S4] = false;
  expecteds[C128][S8] = false;
  expecteds[C128][S16] = false;
  expecteds[C128][S32] = false;
  expecteds[C128][S64] = false;
  expecteds[C128][U4] = false;
  expecteds[C128][U8] = false;
  expecteds[C128][U16] = false;
  expecteds[C128][U32] = false;
  expecteds[C128][U64] = false;
  expecteds[C128][F16] = false;
  expecteds[C128][F32] = false;
  expecteds[C128][F64] = false;
  expecteds[C128][C64] = false;
  expecteds[C128][BF16] = false;
  expecteds[C128][C128] = true;
  expecteds[C128][F8E5M2] = false;
  expecteds[C128][F8E4M3FN] = false;
  expecteds[F8E5M2][PRED] = false;
  expecteds[F8E5M2][S4] = false;
  expecteds[F8E5M2][S8] = false;
  expecteds[F8E5M2][S16] = false;
  expecteds[F8E5M2][S32] = false;
  expecteds[F8E5M2][S64] = false;
  expecteds[F8E5M2][U4] = false;
  expecteds[F8E5M2][U8] = false;
  expecteds[F8E5M2][U16] = false;
  expecteds[F8E5M2][U32] = false;
  expecteds[F8E5M2][U64] = false;
  expecteds[F8E5M2][F16] = true;
  expecteds[F8E5M2][F32] = true;
  expecteds[F8E5M2][F64] = true;
  expecteds[F8E5M2][C64] = true;
  expecteds[F8E5M2][BF16] = true;
  expecteds[F8E5M2][C128] = true;
  expecteds[F8E5M2][F8E5M2] = true;
  expecteds[F8E5M2][F8E4M3FN] = false;
  expecteds[F8E4M3FN][PRED] = false;
  expecteds[F8E4M3FN][S4] = false;
  expecteds[F8E4M3FN][S8] = false;
  expecteds[F8E4M3FN][S16] = false;
  expecteds[F8E4M3FN][S32] = false;
  expecteds[F8E4M3FN][S64] = false;
  expecteds[F8E4M3FN][U4] = false;
  expecteds[F8E4M3FN][U8] = false;
  expecteds[F8E4M3FN][U16] = false;
  expecteds[F8E4M3FN][U32] = false;
  expecteds[F8E4M3FN][U64] = false;
  expecteds[F8E4M3FN][F16] = true;
  expecteds[F8E4M3FN][F32] = true;
  expecteds[F8E4M3FN][F64] = true;
  expecteds[F8E4M3FN][C64] = true;
  expecteds[F8E4M3FN][BF16] = true;
  expecteds[F8E4M3FN][C128] = true;
  expecteds[F8E4M3FN][F8E5M2] = false;
  expecteds[F8E4M3FN][F8E4M3FN] = true;

  for (int from_type_int = PrimitiveType_MIN;
       from_type_int < PrimitiveType_ARRAYSIZE; ++from_type_int) {
    auto from_type = static_cast<PrimitiveType>(from_type_int);
    if (!primitive_util::IsArrayType(from_type)) {
      continue;
    }
    for (int to_type_int = PrimitiveType_MIN;
         to_type_int < PrimitiveType_ARRAYSIZE; ++to_type_int) {
      auto to_type = static_cast<PrimitiveType>(to_type_int);
      if (!primitive_util::IsArrayType(to_type)) {
        continue;
      }
      bool expected = expecteds[from_type][to_type];
      bool actual = primitive_util::CastPreservesValues(from_type, to_type);
      EXPECT_EQ(expected, actual)
          << primitive_util::LowercasePrimitiveTypeName(from_type) << " -> "
          << primitive_util::LowercasePrimitiveTypeName(to_type);
    }
  }
}

}  // namespace
}  // namespace xla
