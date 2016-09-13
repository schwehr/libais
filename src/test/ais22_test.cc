// Test parsing message 22 - Channel Management.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais22> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais22> msg(new Ais22(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais22 *msg,
    const int repeat_indicator,
    const int mmsi,
    const int spare,
    const int chan_a,
    const int chan_b,
    const int txrx_mode,
    const bool power_low,
    const bool pos_valid,
    const double x1,
    const double y1,
    const double x2,
    const double y2,
    const bool dest_valid,
    const int dest_mmsi_1,
    const int dest_mmsi_2,
    const int chan_a_bandwidth,
    const int chan_b_bandwidth,
    const int zone_size,
    const int spare2) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(22, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(spare, msg->spare);
  EXPECT_EQ(chan_a, msg->chan_a);
  EXPECT_EQ(chan_b, msg->chan_b);
  EXPECT_EQ(txrx_mode, msg->txrx_mode);
  EXPECT_EQ(power_low, msg->power_low);
  EXPECT_EQ(pos_valid, msg->pos_valid);
  EXPECT_DOUBLE_EQ(x1, msg->position1.lng_deg);
  EXPECT_DOUBLE_EQ(y1, msg->position1.lat_deg);
  EXPECT_DOUBLE_EQ(x2, msg->position2.lng_deg);
  EXPECT_DOUBLE_EQ(y2, msg->position2.lat_deg);
  EXPECT_EQ(dest_valid, msg->dest_valid);
  EXPECT_EQ(dest_mmsi_1, msg->dest_mmsi_1);
  EXPECT_EQ(dest_mmsi_2, msg->dest_mmsi_2);
  EXPECT_EQ(chan_a_bandwidth, msg->chan_a_bandwidth);
  EXPECT_EQ(chan_b_bandwidth, msg->chan_b_bandwidth);
  EXPECT_EQ(zone_size, msg->zone_size);
  EXPECT_EQ(spare2, msg->spare2);
}

TEST(Ais22Test, DecodeAnything) {
  std::unique_ptr<Ais22> msg = Init(
      "!SAVDM,1,1,6,A,F030owj2N2P6Ubib@=4q35b10000,0*74");

  Validate(
      msg.get(), 0, 3160063, 0,
      2087, 2088, 0, 0,
      true,  // Position valid.
      -77.083333333333329, 45.333333333333336,
      -79.833333333333329, 42.166666666666664,
      false,  // Destination invalid.
      0, 0,
      0, 0, 2, 0);
}

}  // namespace
}  // namespace libais
