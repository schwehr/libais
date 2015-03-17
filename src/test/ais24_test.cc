// Test parsing message 24 - Class B Static Data Report.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais24> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais24> msg(new Ais24(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais24 *msg,
    const int repeat_indicator,
    const int mmsi,
    const int part_num,
    const string name,
    const int type_and_cargo,
    const string vendor_id,
    const string callsign,
    const int dim_a,
    const int dim_b,
    const int dim_c,
    const int dim_d,
    const int spare) {
  ASSERT_NE(nullptr, msg);
  ASSERT_EQ(24, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(part_num, msg->part_num);
  EXPECT_EQ(name, msg->name);
  EXPECT_EQ(type_and_cargo, msg->type_and_cargo);
  EXPECT_EQ(vendor_id, msg->vendor_id);
  EXPECT_EQ(callsign, msg->callsign);
  EXPECT_EQ(dim_a, msg->dim_a);
  EXPECT_EQ(dim_b, msg->dim_b);
  EXPECT_EQ(dim_c, msg->dim_c);
  EXPECT_EQ(dim_d, msg->dim_d);
  EXPECT_EQ(spare, msg->spare);
}

TEST(Ais24Test, DecodeAnything) {
  std::unique_ptr<Ais24> msg = Init(
      "!AIVDM,1,1,,A,H02IDPDm3?=1B00@9<?D00081110,0*6D");

  Validate(
      msg.get(), 0, 2512001, 1, "", 53, "COMAR@@", "PILOT@@", 1, 1, 1, 1, 0);
}

}  // namespace
}  // namespace libais
