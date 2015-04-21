// Test parsing EU River Information System (RIS) 8:200:* messages.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

void Validate8_200_10(const Ais8_200_10 *msg, const int repeat_indicator,
                      const int mmsi, const string &eu_id, const float length,
                      const float beam, const int ship_type,
                      const int haz_cargo, const float draught,
                      const int loaded, const int speed_qual,
                      const int course_qual, const int heading_qual,
                      const int spare2) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(8, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(200, msg->dac);
  ASSERT_EQ(10, msg->fi);

  EXPECT_EQ(length, msg->length);
  EXPECT_EQ(beam, msg->beam);
  EXPECT_EQ(ship_type, msg->ship_type);
  EXPECT_EQ(haz_cargo, msg->haz_cargo);
  EXPECT_EQ(draught, msg->draught);
  EXPECT_EQ(loaded, msg->loaded);
  EXPECT_EQ(speed_qual, msg->speed_qual);
  EXPECT_EQ(course_qual, msg->course_qual);
  EXPECT_EQ(heading_qual, msg->heading_qual);
  EXPECT_EQ(spare2, msg->spare2);
}

TEST(Ais8_200_10Test, DecodeAnything) {
  // clang-format off
  // !SAVDM,1,1,5,B,85NLn@0j2d<8000000BhI?`50000,0*2A,d,S1241,t002333.00,T33.111314,D08MN-NO-BSABS1,1429316613 NOLINT
  // clang-format on

  std::unique_ptr<Ais8_200_10> msg(
      new Ais8_200_10("85NLn@0j2d<8000000BhI?`50000", 0));
  Validate8_200_10(msg.get(), 0, 367474240, string("foo"), 15, 5, 8000, 5,
                   0, 0, 0, 0, 0, 0);
}

}  // namespace
}  // namespace libais
