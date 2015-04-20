// Test parsing 8:366:*.

#include <memory>

#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

void Validate(const Ais8_366_56 *msg, const int repeat_indicator,
              const int mmsi, const vector<unsigned char> &encrypted) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(8, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(366, msg->dac);
  ASSERT_EQ(56, msg->fi);
  EXPECT_THAT(msg->encrypted, testing::ElementsAreArray(encrypted));
}

TEST(Ais8_366_56Test, DecodeAnything) {
  // clang-format off
  // !BSVDM,1,1,,B,853>IhQKf6EQFDdajT?AbaAVhHEWebddhqHC5@?=KwisgP00DWjE,0*6D,b003669701,1429315201  NOLINT
  // clang-format on

  // TODO(schwehr): use make_unique.
  std::unique_ptr<Ais8_366_56> msg(new Ais8_366_56(
      "853>IhQKf6EQFDdajT?AbaAVhHEWebddhqHC5@?=KwisgP00DWjE", 0));

  Validate(msg.get(), 0, 338926018,
           {0x65, 0x61, 0x59, 0x4B, 0x29, 0xCA, 0x43, 0xD1, 0xAA, 0x94, 0x66,
            0xC1, 0x85, 0x67, 0xB6, 0xAB, 0x2C, 0xC3, 0x96, 0x13, 0x15, 0x03,
            0xCD, 0x6F, 0xFC, 0x7B, 0xBE, 0x00, 0x00, 0x52, 0x7C, 0x95});
}

}  // namespace
}  // namespace libais
