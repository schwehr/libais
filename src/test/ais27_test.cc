// Test parsing message 27 - Long-range position report.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais27> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais27> msg(new Ais27(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais27 *msg,
    const int repeat_indicator,
    const int mmsi,
    const int position_accuracy,
    const bool raim,
    const int nav_status,
    const double x,
    const double y,
    const int sog,
    const int cog,
    const bool gnss,
    const int spare) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(27, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(position_accuracy, msg->position_accuracy);
  EXPECT_EQ(raim, msg->raim);
  EXPECT_EQ(nav_status, msg->nav_status);
  EXPECT_DOUBLE_EQ(x, msg->position.lng_deg);
  EXPECT_DOUBLE_EQ(y, msg->position.lat_deg);
  EXPECT_EQ(sog, msg->sog);
  EXPECT_EQ(cog, msg->cog);
  EXPECT_EQ(gnss, msg->gnss);
  EXPECT_EQ(spare, msg->spare);
}

TEST(Ais27Test, DecodeAnything) {
  std::unique_ptr<Ais27> msg = Init(
      "!AIVDM,1,1,,B,K815>P8=5EikdUet,0*6B");

  Validate(
      msg.get(), 0, 538005120, 1, false, 0,
      -79.641666666666666, 24.681666666666668,
      11, 223, true, 0);
}

}  // namespace
}  // namespace libais
