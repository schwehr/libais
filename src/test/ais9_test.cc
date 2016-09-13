// Test parsing message 9 - SAR position report.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais9> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais9> msg(new Ais9(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais9 *msg,
    const int repeat_indicator,
    const int mmsi,
    const int alt,
    const float sog,
    const int position_accuracy,
    const double x,
    const double y,
    const float cog,
    const int timestamp,
    const int alt_sensor,
    const int spare,
    const int dte,
    const int spare2,
    const int assigned_mode,
    const bool raim) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(9, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(alt, msg->alt);
  EXPECT_FLOAT_EQ(sog, msg->sog);
  EXPECT_EQ(position_accuracy, msg->position_accuracy);
  EXPECT_DOUBLE_EQ(x, msg->position.lng_deg);
  EXPECT_DOUBLE_EQ(y, msg->position.lat_deg);
  EXPECT_FLOAT_EQ(cog, msg->cog);
  EXPECT_EQ(timestamp, msg->timestamp);
  EXPECT_EQ(alt_sensor, msg->alt_sensor);
  EXPECT_EQ(spare, msg->spare);
  EXPECT_EQ(dte, msg->dte);
  EXPECT_EQ(spare2, msg->spare2);
  EXPECT_EQ(assigned_mode, msg->assigned_mode);
  EXPECT_EQ(raim, msg->raim);
}

TEST(Ais9Test, DecodeAnything) {
  std::unique_ptr<Ais9> msg = Init(
      "!AIVDM,1,1,,B,9oVAuAI5;rRRv2OqTi?1uoP?=a@1,0*74");

  Validate(
      msg.get(), 3, 509902149, 2324, 762, 1,
      35.601198333333336, -11.229340000000001,
      50.3, 30, 0, 3, 1, 4, 1, true);

  // TODO(schwehr): Validate commstate.
}

}  // namespace
}  // namespace libais
