// Test parsing message 19 "C" - Class B.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

void Validate(const Ais19 *msg, const int repeat_indicator, const int mmsi,
              const int spare, const float sog, const int position_accuracy,
              const float x, const float y, const float cog,
              const int true_heading, const int timestamp, const int spare2,
              const string name, const int type_and_cargo, const int dim_a,
              const int dim_b, const int dim_c, const int dim_d,
              const int fix_type, const bool raim, const int dte,
              const int assigned_mode, const int spare3) {
  ASSERT_NE(nullptr, msg);
  ASSERT_EQ(19, msg->message_id);
  ASSERT_EQ(repeat_indicator, msg->repeat_indicator);
  ASSERT_EQ(mmsi, msg->mmsi);
  ASSERT_EQ(spare, msg->spare);
  ASSERT_EQ(sog, msg->sog);
  ASSERT_EQ(position_accuracy, msg->position_accuracy);
  ASSERT_EQ(x, msg->x);
  ASSERT_EQ(y, msg->y);
  ASSERT_EQ(cog, msg->cog);
  ASSERT_EQ(true_heading, msg->true_heading);
  ASSERT_EQ(timestamp, msg->timestamp);
  ASSERT_EQ(spare2, msg->spare2);
  ASSERT_EQ(name, msg->name);
  ASSERT_EQ(type_and_cargo, msg->type_and_cargo);
  ASSERT_EQ(dim_a, msg->dim_a);
  ASSERT_EQ(dim_b, msg->dim_b);
  ASSERT_EQ(dim_c, msg->dim_c);
  ASSERT_EQ(dim_d, msg->dim_d);
  ASSERT_EQ(fix_type, msg->fix_type);
  ASSERT_EQ(raim, msg->raim);
  ASSERT_EQ(dte, msg->dte);
  ASSERT_EQ(assigned_mode, msg->assigned_mode);
  ASSERT_EQ(spare3, msg->spare3);
}

TEST(Ais19Test, DecodeAnything) {
  // !AIVDM,2,1,7,B,C5NMbDQl0NNJC7VNuC<v`7NF4T28V@2g0J6F::00000,0*59
  // !AIVDM,2,2,7,B,0J70<RRS0,0*30

  std::unique_ptr<Ais19> msg(
      new Ais19("C5NMbDQl0NNJC7VNuC<v`7NF4T28V@2g0J6F::000000J70<RRS0", 0));
  Validate(msg.get(), 0, 367487570, 29 /* spare - suspicious */, 0.1, 1,
           -85.27464294433594, 45.324459075927734, 100.2, 14, 60,
           11 /* spare2 - suspicious */, "BRADSHAW MCKEE@@@@@@", 52, 112, 25, 5,
           5, 1, true, 0, 0, 0);
}

}  // namespace
}  // namespace libais
