// Test parsing message 19 "C" - Class B.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

void Validate(const Ais19 *msg, const int repeat_indicator, const int mmsi,
              const int spare, const float sog, const int position_accuracy,
              const double x, const double y, const float cog,
              const int true_heading, const int timestamp, const int spare2,
              const string name, const int type_and_cargo, const int dim_a,
              const int dim_b, const int dim_c, const int dim_d,
              const int fix_type, const bool raim, const int dte,
              const int assigned_mode, const int spare3) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(19, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(spare, msg->spare);
  EXPECT_FLOAT_EQ(sog, msg->sog);
  EXPECT_EQ(position_accuracy, msg->position_accuracy);
  EXPECT_DOUBLE_EQ(x, msg->position.lng_deg);
  EXPECT_DOUBLE_EQ(y, msg->position.lat_deg);
  EXPECT_FLOAT_EQ(cog, msg->cog);
  EXPECT_EQ(true_heading, msg->true_heading);
  EXPECT_EQ(timestamp, msg->timestamp);
  EXPECT_EQ(spare2, msg->spare2);
  EXPECT_EQ(name, msg->name);
  EXPECT_EQ(type_and_cargo, msg->type_and_cargo);
  EXPECT_EQ(dim_a, msg->dim_a);
  EXPECT_EQ(dim_b, msg->dim_b);
  EXPECT_EQ(dim_c, msg->dim_c);
  EXPECT_EQ(dim_d, msg->dim_d);
  EXPECT_EQ(fix_type, msg->fix_type);
  EXPECT_EQ(raim, msg->raim);
  EXPECT_EQ(dte, msg->dte);
  EXPECT_EQ(assigned_mode, msg->assigned_mode);
  EXPECT_EQ(spare3, msg->spare3);
}

TEST(Ais19Test, DecodeAnything) {
  // !AIVDM,2,1,7,B,C5NMbDQl0NNJC7VNuC<v`7NF4T28V@2g0J6F::00000,0*59
  // !AIVDM,2,2,7,B,0J70<RRS0,0*30

  std::unique_ptr<Ais19> msg(
      new Ais19("C5NMbDQl0NNJC7VNuC<v`7NF4T28V@2g0J6F::000000J70<RRS0", 0));
  Validate(msg.get(), 0, 367487570, 29 /* spare - suspicious */, 0.1, 1,
           -85.274641666666668, 45.324458333333332, 100.2, 14, 60,
           11 /* spare2 - suspicious */, "BRADSHAW MCKEE@@@@@@", 52, 112, 25, 5,
           5, 1, true, 0, 0, 0);
}

}  // namespace
}  // namespace libais
