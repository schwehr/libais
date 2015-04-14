// Test parsing message 14 '>' - Addressd safety related

// TODO(schwehr): Find more test cases.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais14> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais14> msg(new Ais14(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais14 *msg,
    const int repeat_indicator,
    const int mmsi,
    const int spare,
    const string &text) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(14, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(spare, msg->spare);
  EXPECT_EQ(text, msg->text);
}

TEST(Ais14Test, DecodeAnything) {
  std::unique_ptr<Ais14> msg = Init(
      "!AIVDM,1,1,,A,>>M@rl1<59B1@E=@0000000,2*0D");
  Validate(
      msg.get(), 0, 970210000, 0, "SART TEST@@@@@@@");
}

}  // namespace
}  // namespace libais
