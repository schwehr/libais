// Test parsing AIS 6 address binary messages.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

void ValidateAis6(const Ais6 *msg, const int repeat_indicator, const int mmsi,
                  const int seq, const int mmsi_dest, const int retransmit,
                  const int spare, const int dac, const int fi) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  EXPECT_EQ(6, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);

  EXPECT_EQ(seq, msg->seq);
  EXPECT_EQ(mmsi_dest, msg->mmsi_dest);
  EXPECT_EQ(retransmit, msg->retransmit);
  EXPECT_EQ(spare, msg->spare);

  ASSERT_EQ(dac, msg->dac);
  ASSERT_EQ(fi, msg->fi);
}

void ValidateAis6_0_0(const Ais6_0_0 *msg, const int sub_id,
                      const float voltage, const float current,
                      const bool dc_power_supply, const bool light_on,
                      const bool battery_low, const bool off_position,
                      const int spare2) {
  ASSERT_NE(nullptr, msg);
  EXPECT_EQ(sub_id, msg->sub_id);
  EXPECT_EQ(voltage, msg->voltage);
  EXPECT_EQ(current, msg->current);
  EXPECT_EQ(dc_power_supply, msg->dc_power_supply);
  EXPECT_EQ(light_on, msg->light_on);
  EXPECT_EQ(battery_low, msg->battery_low);
  EXPECT_EQ(off_position, msg->off_position);
  EXPECT_EQ(spare2, msg->spare2);
}

// http://www.e-navigation.nl/content/monitoring-aids-navigation
TEST(Ais6_0_0Test, DecodeAnything) {
  // !AIVDM,1,1,,A,6>l4uk@0w2Td000000U00P0,2*5A
  std::unique_ptr<Ais6_0_0> msg(new Ais6_0_0("6>l4uk@0w2Td000000U00P0", 2));
  ValidateAis6(msg.get(), 0, 994131405, 0, 4131403, true, 0, 0, 0);
  ValidateAis6_0_0(msg.get(), 0, 14.8, 0.0, true, false, false, false, 0);
}

TEST(Ais6_0_0Test, DecodeAnything2) {
  // !AIVDM,1,1,,A,6>l4v:h0006D000000P@0T0,2*56
  std::unique_ptr<Ais6_0_0> msg(new Ais6_0_0("6>l4v:h0006D000000P@0T0", 2));
  ValidateAis6(msg.get(), 0, 994131499, 0, 101, true, 0, 0, 0);
  ValidateAis6_0_0(msg.get(), 0, 12.9, 0.0, true, false, false, true, 0);
}

TEST(Ais6_0_0Test, DecodeAnything3) {
  // !AIVDM,1,1,,A,6>l4ukP0w2Td000000W00P0,2*48
  std::unique_ptr<Ais6_0_0> msg(new Ais6_0_0("6>l4ukP0w2Td000000W00P0", 2));
  ValidateAis6(msg.get(), 0, 994131406, 0, 4131403, true, 0, 0, 0);
  ValidateAis6_0_0(msg.get(), 0, 15.6, 0.0, true, false, false, false, 0);
}

void ValidateAis6_1_0(const Ais6_1_0 *msg, const bool ack_required,
                      const int msg_seq, const string &text, const int spare2) {
  ASSERT_NE(nullptr, msg);
  EXPECT_EQ(ack_required, msg->ack_required);
  EXPECT_EQ(msg_seq, msg->msg_seq);
  EXPECT_EQ(text, msg->text);
  EXPECT_EQ(spare2, msg->spare2);
}

TEST(Ais6_1_0Test, DecodeAnything) {
  // !AIVDM,1,1,,B,65Ps:8=:0MjP0420<4U>1@E=B10i>04<fp0,2*23
  std::unique_ptr<Ais6_1_0> msg(
      new Ais6_1_0("65Ps:8=:0MjP0420<4U>1@E=B10i>04<fp0", 2));
  ValidateAis6(msg.get(), 0, 370068000, 3, 310409000, true, 0, 1, 0);
  ValidateAis6_1_0(msg.get(), true, 3, "AIS TEST PLS ACK.@", 0);
}

// TODO(schwehr): Test Ais6_1_1.
// TODO(schwehr): Test Ais6_1_2.
// TODO(schwehr): Test Ais6_1_3.
// TODO(schwehr): Test Ais6_1_4.
// TODO(schwehr): Test Ais6_1_5.
// TODO(schwehr): Test Ais6_1_12.
// TODO(schwehr): Test Ais6_1_14.
// TODO(schwehr): Test Ais6_1_18.
// TODO(schwehr): Test Ais6_1_20.
// TODO(schwehr): Test Ais6_1_25.
// TODO(schwehr): Test Ais6_1_32.
// TODO(schwehr): Test Ais6_1_40.

}  // namespace
}  // namespace libais
