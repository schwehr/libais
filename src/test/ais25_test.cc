// Test parsing message 25 - Single slot binary message.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais25> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais25> msg(new Ais25(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais25 *msg,
    const int repeat_indicator,
    const int mmsi,
    const bool use_app_id,
    const bool dest_mmsi_valid,
    const int dest_mmsi,
    const int dac,
    const int fi) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(25, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(use_app_id, msg->use_app_id);
  EXPECT_EQ(dest_mmsi_valid, msg->dest_mmsi_valid);
  EXPECT_EQ(dest_mmsi, msg->dest_mmsi);
  EXPECT_EQ(dac, msg->dac);
  EXPECT_EQ(fi, msg->fi);
}

TEST(Ais25Test, DecodeAnything) {
  std::unique_ptr<Ais25> msg = Init(
      "!AIVDM,1,1,,B,I6S`3Tg@T0a3REBEsjJcT?wSi0fM,0*02");

  Validate(
      msg.get(), 0, 440009618, true, true, 874775184, 905, 21);
}

}  // namespace
}  // namespace libais
