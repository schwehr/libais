// Test parsing message 21 - ATON Status.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

void Validate(
    const Ais21 *msg,
    const int repeat_indicator,
    const int mmsi,
    const int aton_type,
    const string name,
    const int position_accuracy,
    const float x,
    const float y,
    const int dim_a,
    const int dim_b,
    const int dim_c,
    const int dim_d,
    const int fix_type,
    const int timestamp,
    const bool off_pos,
    const int aton_status,
    const bool raim,
    const bool virtual_aton,
    const bool assigned_mode,
    const int spare,
    const int spare2) {
  ASSERT_NE(nullptr, msg);
  ASSERT_EQ(21, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(aton_type, msg->aton_type);
  EXPECT_EQ(name, msg->name);
  EXPECT_EQ(position_accuracy, msg->position_accuracy);
  EXPECT_EQ(x, msg->x);
  EXPECT_EQ(y, msg->y);
  EXPECT_EQ(dim_a, msg->dim_a);
  EXPECT_EQ(dim_b, msg->dim_b);
  EXPECT_EQ(dim_c, msg->dim_c);
  EXPECT_EQ(dim_d, msg->dim_d);
  EXPECT_EQ(fix_type, msg->fix_type);
  EXPECT_EQ(timestamp, msg->timestamp);
  EXPECT_EQ(off_pos, msg->off_pos);
  EXPECT_EQ(aton_status, msg->aton_status);
  EXPECT_EQ(raim, msg->raim);
  EXPECT_EQ(virtual_aton, msg->virtual_aton);
  EXPECT_EQ(assigned_mode, msg->assigned_mode);
  EXPECT_EQ(spare, msg->spare);
  EXPECT_EQ(spare2, msg->spare2);
}

TEST(Ais21Test, DecodeAnything) {
  // !SAVDM,1,1,0,B,Evkb9Mq1WV:VQ4Ph94c@6;Q@1a@;ShvA==bd`00003vP000,2*65
  std::unique_ptr<Ais21> msg(new Ais21(
      "Evkb9Mq1WV:VQ4Ph94c@6;Q@1a@;ShvA==bd`00003vP000", 2));

  Validate(
      msg.get(), 3, 993692023, 18, "COLUMBIA RIV LWB CR @", 0,
      -124.18408203125, 46.18470001220703, 0, 0, 0, 0, 7, 61, false,
      0, false, false, false, 0, 0);
}

TEST(Ais21Test, TwoLine) {
  // !AIVDM,2,1,9,A,ENk`sO70VQ97aRh1T0W72V@611@=FVj<;V5d@00003v,0*50
  // !AIVDM,2,2,9,A,P0<M0,0*3E
  std::unique_ptr<Ais21> msg(new Ais21(
      "ENk`sO70VQ97aRh1T0W72V@611@=FVj<;V5d@00003vP0<M0", 0));

  Validate(
      msg.get(), 1, 993672060, 14, "AMBROSE CHANNEL LBB 14", 0,
      -74.00936889648438, 40.527950286865234, 0, 0, 0, 0, 7, 61, false,
      0, false, false, false, 0, 0);
}

}  // namespace
}  // namespace libais