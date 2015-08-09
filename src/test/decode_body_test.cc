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
