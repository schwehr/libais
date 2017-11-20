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
  EXPECT_FLOAT_EQ(voltage, msg->voltage);
  EXPECT_FLOAT_EQ(current, msg->current);
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

TEST(Ais6_1_0Test, b69511430TooFewBits) {
  std::unique_ptr<Ais6_1_0> msg(new Ais6_1_0("6801tME4j60E041tM", 3));
  EXPECT_EQ(AIS_ERR_BAD_BIT_COUNT, msg->get_error());
}

// TODO(schwehr): Test Ais6_1_1.
// TODO(schwehr): Test Ais6_1_2.
// TODO(schwehr): Test Ais6_1_3.
// TODO(schwehr): Test Ais6_1_4.

void ValidateAis6_1_5(const Ais6_1_5 &msg, int seq_num, bool ai_available,
                      int ai_response, int spare, int spare2) {
  ASSERT_EQ(AIS_OK, msg.get_error());
  EXPECT_EQ(1, msg.dac);
  EXPECT_EQ(5, msg.fi);
  EXPECT_EQ(seq_num, msg.seq_num);
  EXPECT_EQ(ai_available, msg.ai_available);
  EXPECT_EQ(ai_response, msg.ai_response);
  EXPECT_EQ(spare, msg.spare);
  EXPECT_EQ(spare2, msg.spare2);
}

TEST(Ais6_1_5, Issue162) {
  // 24804348,s:rORBCOMM999,q:u,c:1509794887,T:2017-11-04 11.28.07*59
  //   !AIVDM,1,1,,B,677IKl=HQGw004D0@02000000000,0*5B
  std::unique_ptr<Ais6_1_5> msg(
      new Ais6_1_5("677IKl=HQGw004D0@02000000000", 0));
  ValidateAis6_1_5(*msg, 4, false, 0, 0, 0);

  // 24804513,s:rORBCOMM999,q:u,c:1509800116,T:2017-11-04 12.55.16*53
  //   !AIVDM,1,1,,B,69NSH@AI746004D0@06B00000000,0*46
  msg.reset(new Ais6_1_5("69NSH@AI746004D0@06B00000000", 0));
  ValidateAis6_1_5(*msg, 12, true, 1, 0, 0);

  // 11563363,s:rORBCOMM999,q:u,c:1509933658,T:2017-11-06 02.00.58*56
  //   !AIVDM,1,1,,A,676ur660AE4F04D0@0;P00000000,0*3B
  msg.reset(new Ais6_1_5("676ur660AE4F04D0@0;P00000000", 0));
  ValidateAis6_1_5(*msg, 23, false, 0, 0, 0);

  // 23545620,s:rORBCOMM000,q:u,c:1510271386,T:2017-11-09 23.49.46*52
  //   !AIVDM,1,1,,A,69NSKE20ACVL04D0@03P00000000,0*43
  msg.reset(new Ais6_1_5("69NSKE20ACVL04D0@03P00000000", 0));
  ValidateAis6_1_5(*msg, 7, false, 0, 0, 0);
}

// TODO(schwehr): Test Ais6_1_12.
// TODO(schwehr): Test Ais6_1_14.
// TODO(schwehr): Test Ais6_1_18.
// TODO(schwehr): Test Ais6_1_20.
// TODO(schwehr): Test Ais6_1_25.
// TODO(schwehr): Test Ais6_1_32.
// TODO(schwehr): Test Ais6_1_40.

}  // namespace
}  // namespace libais
