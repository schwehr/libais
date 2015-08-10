// Copyright 2015 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "decode_body.h"
#include "gtest/gtest.h"

namespace libais {
namespace {

TEST(CreateAisMsgTest, Invalid) {
  EXPECT_EQ(nullptr, CreateAisMsg("", 0).get());
  EXPECT_EQ(nullptr, CreateAisMsg("L", 0).get());
  EXPECT_EQ(nullptr, CreateAisMsg("Z", 0).get());
  EXPECT_EQ(nullptr, CreateAisMsg("a", 0).get());
  EXPECT_EQ(nullptr, CreateAisMsg("z", 0).get());
}

TEST(CreateAisMsgTest, Valid) {
  // !SAVDM,1,1,,B,K8VSqb9LdU28WP7h,0*4C
  auto msg = CreateAisMsg("K8VSqb9LdU28WP7h", 0);
  ASSERT_NE(nullptr, msg);
  EXPECT_EQ(27, msg->message_id);
}

}  // namespace
}  // namespace libais
