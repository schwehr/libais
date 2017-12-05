// Test parsing US Coast Guard (USCG) 8:367:* messages.

// TODO(schwehr): Try these message.  Some may crash libais.
// clang-format off
// 8 367 22 3669732 !SAVDO,1,1,2,A,8h3Ovq1KmPAc08aTH07P3cmt8IPq:?Akh000,0*0E,b003669732,1428669559 NOLINT
// 8 367 22 3669732 !ANVDM,1,1,,A,8h3Ovq1KmPAc08aTH07P3cmt8IPq:?Akh000,0*23,r08ACERDC,1428669557 NOLINT
// 8 367 22 3669732 !SAVDO,1,1,7,B,8h3Ovq1KmPAd0``002l03ckq=qPr=MAkh000,0*3B,b003669732,1428669564 NOLINT
// 8 367 22 3669732 !ANVDM,1,1,,B,8h3Ovq1KmPAd0``002l03ckq=qPr=MAkh000,0*13,r08ACERDC,1428669562 NOLINT
// 8 367 22 3669732 !AIVDM,1,1,,B,8h3Ovq1KmPAd0``002l03ckq=qPr=MAkh000,0*14,rZINHAN1,1428669568 NOLINT
// 8 367 22 3669732 !SAVDO,1,1,3,A,8h3Ovq1KmPHu08aTp3oh1cG=91LUBh@00000T02upLGve2us@000,0*49,b003669732,1428669599 NOLINT
// 8 367 22 3669732 !ANVDM,1,1,,A,8h3Ovq1KmPHu08aTp3oh1cG=91LUBh@00000T02upLGve2us@000,0*65,r08ACERDC,1428669597 NOLINT
// 8 367 22 3669732 !SAVDO,1,1,1,B,8h3Ovq1KmPHw08aTp?IH1chmi1Md2p@00000T02v8LGle2v;@000,0*2D,b003669732,1428669704 NOLINT
// 8 367 22 3669732 !AIVDM,1,1,,B,8h3Ovq1KmPHw08aTp?IH1chmi1Md2p@00000T02v8LGle2v;@000,0*04,rZINHAN1,1428669705 NOLINT
// 8 367 22 3669732 !ANVDM,1,1,,B,8h3Ovq1KmPHw08aTp?IH1chmi1Md2p@00000T02v8LGle2v;@000,0*03,r08ACERDC,1428669702 NOLINT
// clang-format on

#include <memory>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais8_367_22> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais8_367_22> msg(new Ais8_367_22(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(const Ais8_367_22 *msg, const int repeat_indicator,
              const int mmsi, const int link_id, const int notice_type,
              const int month, const int day, const int hour, const int minute,
              const int duration_minutes) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(8, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(367, msg->dac);
  ASSERT_EQ(22, msg->fi);

  EXPECT_EQ(0, msg->spare);

  EXPECT_EQ(link_id, msg->link_id);
  EXPECT_EQ(notice_type, msg->notice_type);
  EXPECT_EQ(month, msg->month);
  EXPECT_EQ(day, msg->day);
  EXPECT_EQ(hour, msg->hour);
  EXPECT_EQ(minute, msg->minute);
  EXPECT_EQ(duration_minutes, msg->duration_minutes);
}

void ValidateCircle(const Ais8_367_22_SubArea *sub_area,
                    const AisPoint position, const int precision,
                    const int radius_m, const unsigned int spare) {
  ASSERT_EQ(AIS8_366_22_SHAPE_CIRCLE, sub_area->getType());
  auto shape = dynamic_cast<const Ais8_367_22_Circle *>(sub_area);
  EXPECT_NEAR(position.lng_deg, shape->position.lng_deg, 0.001);
  EXPECT_NEAR(position.lat_deg, shape->position.lat_deg, 0.001);
  EXPECT_EQ(precision, shape->precision);
  EXPECT_EQ(radius_m, shape->radius_m);
  EXPECT_EQ(spare, shape->spare);
}

// TODO(schwehr): Test area_type.
void ValidatePoly(const Ais8_367_22_SubArea *sub_area,
                  const Ais8_366_22_AreaShapeEnum /* area_type */,
                  const vector<float> &angles, const vector<float> &dists_m,
                  const unsigned int spare) {
  ASSERT_TRUE(AIS8_366_22_SHAPE_POLYLINE == sub_area->getType() ||
              AIS8_366_22_SHAPE_POLYGON == sub_area->getType());
  auto shape = dynamic_cast<const Ais8_367_22_Poly *>(sub_area);
  ASSERT_EQ(angles.size(), dists_m.size());
  EXPECT_THAT(shape->angles, testing::ElementsAreArray(angles));
  EXPECT_THAT(shape->dists_m, testing::ElementsAreArray(dists_m));
  EXPECT_EQ(spare, shape->spare);
}

TEST(Ais8_367_22Test, DecodeSingleTest) {
  // Invalid USCG test message with only a TEXT subpacket.
  // The parser allows it anyway and assumes that the caller will validate.
  // ,b2003669952,1413914824
  std::unique_ptr<Ais8_367_22> msg =
      Init("!AIVDM,1,1,,A,803Ow2iKmPFJwP37P000bbHHsrPbJP000000,0*6E");
  Validate(msg.get(), 0, 3669771, 410, 127, 0, 0, 24, 60, 0);

  ASSERT_EQ(1, msg->sub_areas.size());

  ASSERT_EQ(AIS8_366_22_SHAPE_TEXT, msg->sub_areas[0]->getType());

  Ais8_367_22_Text *text = dynamic_cast<Ais8_367_22_Text *>(msg->sub_areas[0]);

  EXPECT_EQ("USCG-TEST@@@@@@", text->text);
  EXPECT_EQ(0, text->spare);
}

TEST(Ais8_367_22Test, DecodeUscgWhaleBouyTest) {
  // !AIVDM,1,1,,B,8h3Ovq1KmPA`08b8007P3ct5uAPmtlAkh000,0*2F,rZINHAN1,1428685389
  std::unique_ptr<Ais8_367_22> msg(
      new Ais8_367_22("8h3Ovq1KmPA`08b8007P3ct5uAPmtlAkh000", 0));
  Validate(msg.get(), 3, 3669732, 104, 0, 4, 10, 17, 0, 60);

  ASSERT_EQ(1, msg->sub_areas.size());

  ASSERT_EQ(AIS8_366_22_SHAPE_CIRCLE, msg->sub_areas[0]->getType());

  Ais8_367_22_Circle *circle =
      dynamic_cast<Ais8_367_22_Circle *>(msg->sub_areas[0]);
  EXPECT_NEAR(-70.1184, circle->position.lng_deg, 0.0001);
  EXPECT_NEAR(42.3113, circle->position.lat_deg, 0.0001);
  EXPECT_EQ(2, circle->precision);
  EXPECT_EQ(9260, circle->radius_m);
  EXPECT_EQ(0, circle->spare);
}

TEST(Ais8_367_22Test, PolylinesTest) {
  // clang-format off
  // !ANVDM,2,1,0,B,8h3Ovq1KmP@N<95=`2l01=dN<b7pGeP00000LL8PSV8RQ8cTs5H0LTHh477P,0*36,r08ACERDC,1429315382 NOLINT
  // !ANVDM,2,2,0,B,Rpus@000,0*46,r08ACERDC,1429315382
  // clang-format on

  std::unique_ptr<Ais8_367_22> msg(new Ais8_367_22(
      "8h3Ovq1KmP@N<95=`2l01=dN<b7pGeP00000LL8PSV8RQ8cTs5H0LTHh477PRpus@000",
      0));
  Validate(msg.get(), 3, 3669732, 30, 24, 4, 17, 9, 45, 1440);

  ASSERT_EQ(3, msg->sub_areas.size());

  ASSERT_EQ(AIS8_366_22_SHAPE_CIRCLE, msg->sub_areas[0]->getType());
  ASSERT_EQ(AIS8_366_22_SHAPE_POLYLINE, msg->sub_areas[1]->getType());
  ASSERT_EQ(AIS8_366_22_SHAPE_POLYLINE, msg->sub_areas[2]->getType());

  ValidateCircle(msg->sub_areas[0], {-175.829, 59.3672}, 4, 0, 0);
  ValidatePoly(msg->sub_areas[1], AIS8_366_22_SHAPE_POLYLINE,
               {225, 230, 265, 315}, {13000, 27300, 17400, 17200}, 0);
  ValidatePoly(msg->sub_areas[2], AIS8_366_22_SHAPE_POLYLINE, {291, 263, 279},
               {19200, 24000, 24700}, 0);
}

TEST(Ais8_367_22Test, BadBitsAtEnd) {
  std::unique_ptr<Ais8_367_22> msg(new Ais8_367_22(
      "8888tMEKmblAb@IIIIIIIIQIIF20DH1b60bbd7l0<50@kk2Q5h@0010N000000<50@"
      "kk2K5hQ5h@@0000N000000",
      2));
  ASSERT_TRUE(msg->had_error());
}

#if 0
// TODO: Get this to work.
TEST(Ais8_367_22Test, DecodeUscgWhaleBouyTest2) {
  // clang-format off
  // !ANVDM,1,1,,A,8h3Ovq1KmPHu08aTp3oh1cG=91LUBh@00000T02upLGve2us@000,0*65,r08ACERDC,1428669597 NOLINT
  // clang-format on
  std::unique_ptr<Ais8_367_22> msg(new Ais8_367_22(
      "8h3Ovq1KmPHu08aTp3oh1cG=91LUBh@00000T02upLGve2us@000", 0));
  Validate(msg.get(), 3, 3669732, 104, 0, 4, 10, 17, 0, 60);

  ASSERT_EQ(1, msg->sub_areas.size());

  ASSERT_EQ(AIS8_366_22_SHAPE_CIRCLE, msg->sub_areas[0]->getType());

  Ais8_367_22_Circle *circle =
      dynamic_cast<Ais8_367_22_Circle *>(msg->sub_areas[0]);
  EXPECT_NEAR(-70.1184, circle->position.lng_deg, 0.0001);
  EXPECT_NEAR(42.3113, circle->position.lat_deg, 0.0001);
  EXPECT_EQ(2, circle->precision);
  EXPECT_EQ(9260, circle->radius_m);
  EXPECT_EQ(0, circle->spare);
}
#endif

}  // namespace
}  // namespace libais
