// Test parsing message 5 - Class A static data.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

void Validate(const Ais5 *msg, const int repeat_indicator, const int mmsi,
              const int ais_version, const int imo_num, const string callsign,
              const string name, const int type_and_cargo, const int dim_a,
              const int dim_b, const int dim_c, const int dim_d,
              const int fix_type, const int eta_month, const int eta_day,
              const int eta_hour, const int eta_minute, const float draught,
              const string destination, const int dte, const int spare) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(5, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(ais_version, msg->ais_version);
  EXPECT_EQ(imo_num, msg->imo_num);
  EXPECT_EQ(callsign, msg->callsign);
  EXPECT_EQ(name, msg->name);
  EXPECT_EQ(type_and_cargo, msg->type_and_cargo);
  EXPECT_EQ(dim_a, msg->dim_a);
  EXPECT_EQ(dim_b, msg->dim_b);
  EXPECT_EQ(dim_c, msg->dim_c);
  EXPECT_EQ(dim_d, msg->dim_d);
  EXPECT_EQ(fix_type, msg->fix_type);
  EXPECT_EQ(eta_month, msg->eta_month);
  EXPECT_EQ(eta_day, msg->eta_day);
  EXPECT_EQ(eta_hour, msg->eta_hour);
  EXPECT_EQ(eta_minute, msg->eta_minute);
  EXPECT_FLOAT_EQ(draught, msg->draught);
  EXPECT_EQ(destination, msg->destination);
  EXPECT_EQ(dte, msg->dte);
  EXPECT_EQ(spare, msg->spare);
}

TEST(Ais5Test, DecodeAnything) {
  // clang-format off
  // !SAVDM,2,1,6,A,55NOvQP1u>QIL@O??SL985`u>0EQ18E=>222221J1p`884i6N344Sll1@m80,0*0C,b003669956,1426118503 NOLINT
  // !SAVDM,2,2,6,A,TRA1iH88880,2*6F,b003669956,1426118503
  // clang-format on

  std::unique_ptr<Ais5> msg(new Ais5(
      "55NOvQP1u>QIL@O??SL985`u>0EQ18E=>222221J1p`884i6N344Sll1@m80"
      "TRA1iH88880",
      2));
  Validate(msg.get(), 0, 367525510, 0, 8206870, "WDG3387",
           "BRAZOS EXPRESS      ", 90, 15, 40, 8, 8, 1, 3, 2, 6, 30, 1.2,
           "PROSPECT BRIDGE     ", 0, 0);
}

}  // namespace
}  // namespace libais
