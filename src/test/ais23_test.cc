// Test parsing message 23 - Group Assignment Command.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais23> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais23> msg(new Ais23(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais23 *msg,
    const int repeat_indicator,
    const int mmsi,
    const int spare,
    const double x1,
    const double y1,
    const double x2,
    const double y2,
    const int station_type,
    const int type_and_cargo,
    const int spare2,
    const int txrx_mode,
    const int interval_raw,
    const int quiet,
    const int spare3) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(23, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(spare, msg->spare);
  EXPECT_DOUBLE_EQ(x1, msg->position1.lng_deg);
  EXPECT_DOUBLE_EQ(y1, msg->position1.lat_deg);
  EXPECT_DOUBLE_EQ(x2, msg->position2.lng_deg);
  EXPECT_DOUBLE_EQ(y2, msg->position2.lat_deg);
  EXPECT_EQ(station_type, msg->station_type);
  EXPECT_EQ(type_and_cargo, msg->type_and_cargo);
  EXPECT_EQ(spare2, msg->spare2);
  EXPECT_EQ(txrx_mode, msg->txrx_mode);
  EXPECT_EQ(interval_raw, msg->interval_raw);
  EXPECT_EQ(quiet, msg->quiet);
  EXPECT_EQ(spare3, msg->spare3);
}

TEST(Ais23Test, DecodeAnything) {
  std::unique_ptr<Ais23> msg = Init(
      "!AIVDM,1,1,,B,G02:KpP1R`sn@291njF00000900,2*1C");

  Validate(
      msg.get(), 0, 2268130, 0,
      2.63, 51.07,
      1.8266666666666667, 50.681666666666665,
      6, 0, 0, 0, 9, 0, 0);
}

}  // namespace
}  // namespace libais
