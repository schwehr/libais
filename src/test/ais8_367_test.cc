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
#include <string>

#include "ais.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace libais {
namespace {

std::unique_ptr<Ais8_367_22> Init(const std::string &nmea_string) {
  const std::string body(GetBody(nmea_string));
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
                  const std::vector<float> &angles, const std::vector<float> &dists_m,
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

  Ais8_367_22_Text *text =
    dynamic_cast<Ais8_367_22_Text *>(msg->sub_areas[0].get());

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
      dynamic_cast<Ais8_367_22_Circle *>(msg->sub_areas[0].get());
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

  ValidateCircle(msg->sub_areas[0].get(), {-175.829, 59.3672}, 4, 0, 0);
  ValidatePoly(msg->sub_areas[1].get(), AIS8_366_22_SHAPE_POLYLINE,
               {225, 230, 265, 315}, {13000, 27300, 17400, 17200}, 0);
  ValidatePoly(msg->sub_areas[2].get(), AIS8_366_22_SHAPE_POLYLINE, {291, 263, 279},
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

std::unique_ptr<Ais8_367_23> Init_23(const std::string &nmea_string) {
  const std::string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais8_367_23> msg(new Ais8_367_23(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }

  return msg;
}

TEST(Ais8_367_23Test, DecodeSingleTest_23) {
  std::unique_ptr<Ais8_367_23> msg =
      Init_23("!SAVDM,1,1,7,A,85Oqf`1Kmh37VDgTDMsgo9@0uNc@,0*47");

  ASSERT_EQ(8, msg->message_id);
  ASSERT_EQ(0, msg->repeat_indicator);
  ASSERT_EQ(368996000, msg->mmsi);
  ASSERT_EQ(0, msg->spare);
  ASSERT_EQ(367, msg->dac);
  ASSERT_EQ(23, msg->fi);

  ASSERT_EQ(0, msg->version);
  ASSERT_EQ(0, msg->utc_day);
  ASSERT_EQ(24, msg->utc_hour);
  ASSERT_EQ(60, msg->utc_min);
  EXPECT_NEAR(msg->position.lng_deg, -117.15298333333334, 0.001);
  EXPECT_NEAR(msg->position.lat_deg, 32.69541666666667, 0.001);
  ASSERT_EQ(1201, msg->pressure);

  ASSERT_EQ(-1024, msg->air_temp_raw);
  EXPECT_NEAR(msg->air_temp, -102.4, 0.001);

  ASSERT_EQ(122, msg->wind_speed);
  ASSERT_EQ(122, msg->wind_gust);
  ASSERT_EQ(360, msg->wind_dir);

  ASSERT_EQ(0, msg->spare2);
}

std::unique_ptr<Ais8_367_24> Init_24(const std::string &nmea_string) {
  const std::string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais8_367_24> msg(new Ais8_367_24(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }

  return msg;
}

TEST(Ais8_367_24Test, DecodeSingleTest_24) {
  std::unique_ptr<Ais8_367_24> msg =
      Init_24("!AIVDM,1,1,,,85MU:l1Kn12FsqBb>Kh@p0,4*4E");

  ASSERT_EQ(8, msg->message_id);
  ASSERT_EQ(0, msg->repeat_indicator);
  ASSERT_EQ(366562000, msg->mmsi);
  ASSERT_EQ(0, msg->spare);
  ASSERT_EQ(367, msg->dac);
  ASSERT_EQ(24, msg->fi);

  ASSERT_EQ(0, msg->version);
  ASSERT_EQ(16, msg->utc_hour);
  ASSERT_EQ(37, msg->utc_min);
  EXPECT_NEAR(msg->position.lng_deg, -148.77725, 0.001);
  EXPECT_NEAR(msg->position.lat_deg, 31.5308, 0.001);

  ASSERT_EQ(1023, msg->pressure);
}

std::unique_ptr<Ais8_367_25> Init_25(const std::string &nmea_string) {
  const std::string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais8_367_25> msg(new Ais8_367_25(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }

  return msg;
}

TEST(Ais8_367_25Test, DecodeSingleTest_25) {
  std::unique_ptr<Ais8_367_25> msg =
      Init_25("!SAVDM,1,1,7,A,85MrbQ1KnA5IdV24,0*17");

  ASSERT_EQ(8, msg->message_id);
  ASSERT_EQ(0, msg->repeat_indicator);
  ASSERT_EQ(366914180, msg->mmsi);
  ASSERT_EQ(0, msg->spare);
  ASSERT_EQ(367, msg->dac);
  ASSERT_EQ(25, msg->fi);

  ASSERT_EQ(0, msg->version);
  ASSERT_EQ(17, msg->utc_hour);
  ASSERT_EQ(22, msg->utc_min);

  ASSERT_EQ(1016, msg->pressure);
  ASSERT_EQ(24, msg->wind_speed);
  ASSERT_EQ(66, msg->wind_dir);
}

std::unique_ptr<Ais8_367_33> Init_33(const std::string &nmea_string) {
  const std::string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  std::unique_ptr<Ais8_367_33> msg(new Ais8_367_33(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }

  return msg;
}

TEST(Ais8_367_33Test, DecodeSingleTest_33) {
  std::unique_ptr<Ais8_367_33> msg =
      Init_33("!AIVDM,1,1,,B,8>k1oFAKpB95?AruFRl7mre0<N00,0*6A");

  ASSERT_NE(nullptr, msg);

  ASSERT_EQ(8, msg->message_id);
  ASSERT_EQ(0, msg->repeat_indicator);
  ASSERT_EQ(993032025, msg->mmsi);
  ASSERT_EQ(0, msg->spare);
  ASSERT_EQ(367, msg->dac);
  ASSERT_EQ(33, msg->fi);

  ASSERT_EQ(1, msg->reports.size());

  ASSERT_EQ(AIS8_367_33_SENSOR_WIND, msg->reports[0]->getType());

  Ais8_367_33_Wind *wind =
    dynamic_cast<Ais8_367_33_Wind *>(msg->reports[0].get());

  EXPECT_EQ(4, wind->utc_day);
  EXPECT_EQ(17, wind->utc_hr);
  EXPECT_EQ(19, wind->utc_min);
  EXPECT_EQ(104, wind->site_id);

  EXPECT_EQ(122, wind->wind_speed);
  EXPECT_EQ(122, wind->wind_gust);
  EXPECT_EQ(360, wind->wind_dir);

  EXPECT_EQ(1, wind->sensor_type);

  EXPECT_EQ(122, wind->wind_forecast);
  EXPECT_EQ(122, wind->wind_gust_forecast);
  EXPECT_EQ(360, wind->wind_dir_forecast);

  EXPECT_EQ(0, wind->utc_day_forecast);
  EXPECT_EQ(24, wind->utc_hour_forecast);
  EXPECT_EQ(60, wind->utc_min_forecast);

  ASSERT_EQ(0, wind->duration);
  EXPECT_EQ(0, wind->spare2);
}

TEST(Ais8_367_33Test, DecodeSingleTest_1_33) {
  std::unique_ptr<Ais8_367_33> msg =
      Init_33("!AIVDM,2,1,6,,85362R1Kp@HpL07cebNpkUqR`O`0USQh17CvENUfI6@0002n>703wA937cmJ<N0000,4*54");

  ASSERT_NE(nullptr, msg);

  ASSERT_EQ(8, msg->message_id);
  ASSERT_EQ(0, msg->repeat_indicator);
  ASSERT_EQ(338789000, msg->mmsi);
  ASSERT_EQ(0, msg->spare);
  ASSERT_EQ(367, msg->dac);
  ASSERT_EQ(33, msg->fi);

  ASSERT_EQ(3, msg->reports.size());

  // Location
  ASSERT_EQ(AIS8_367_33_SENSOR_LOCATION, msg->reports[0]->getType());

  Ais8_367_33_Location *loc =
    dynamic_cast<Ais8_367_33_Location *>(msg->reports[0].get());

  EXPECT_EQ(12, loc->utc_day);
  EXPECT_EQ(14, loc->utc_hr);
  EXPECT_EQ(7, loc->utc_min);
  EXPECT_EQ(0, loc->site_id);

  EXPECT_EQ(3, loc->version);

  EXPECT_NEAR(loc->position.lng_deg, -70.9065, 0.0001);
  EXPECT_NEAR(loc->position.lat_deg, 22.541, 0.0001);

  EXPECT_EQ(5, loc->precision);
  EXPECT_NEAR(loc->altitude, 25.3, 0.001);
  EXPECT_EQ(0, loc->owner);
  EXPECT_EQ(0, loc->spare2);

  // Weather
  ASSERT_EQ(AIS8_367_33_SENSOR_WX, msg->reports[1]->getType());

  Ais8_367_33_Wx *wx =
    dynamic_cast<Ais8_367_33_Wx *>(msg->reports[1].get());

  EXPECT_EQ(12, wx->utc_day);
  EXPECT_EQ(14, wx->utc_hr);
  EXPECT_EQ(7, wx->utc_min);
  EXPECT_EQ(0, wx->site_id);

  EXPECT_NEAR(wx->air_temp, 28.5, 0.001);
  EXPECT_EQ(1, wx->air_temp_sensor_type);
  EXPECT_EQ(3, wx->precip);
  EXPECT_NEAR(wx->horz_vis, 24.2, 0.001);
  EXPECT_NEAR(wx->dew_point, 50.1, 0.001);
  EXPECT_EQ(1, wx->dew_point_type);
  EXPECT_EQ(1019, wx->air_pressure);
  EXPECT_EQ(3, wx->air_pressure_trend);
  EXPECT_EQ(1, wx->air_pressure_sensor_type);
  EXPECT_NEAR(wx->salinity, 5.0, 0.001);
  EXPECT_EQ(0, wx->spare2);

  // Wind Report (V2)
  ASSERT_EQ(AIS8_367_33_SENSOR_WIND_REPORT_2, msg->reports[2]->getType());

  Ais8_367_33_Wind_V2 *wind_v2 =
    dynamic_cast<Ais8_367_33_Wind_V2 *>(msg->reports[2].get());

  EXPECT_EQ(12, wind_v2->utc_day);
  EXPECT_EQ(14, wind_v2->utc_hr);
  EXPECT_EQ(7, wind_v2->utc_min);
  EXPECT_EQ(0, wind_v2->site_id);

  EXPECT_EQ(15, wind_v2->wind_speed);
  EXPECT_EQ(122, wind_v2->wind_gust);
  EXPECT_EQ(73, wind_v2->wind_dir);
  EXPECT_EQ(3, wind_v2->averaging_time);
  EXPECT_EQ(0, wind_v2->sensor_type);
  EXPECT_EQ(122, wind_v2->wind_speed_forecast);
  EXPECT_EQ(122, wind_v2->wind_gust_forecast);
  EXPECT_EQ(360, wind_v2->wind_dir_forecast);
  EXPECT_EQ(24, wind_v2->utc_hour_forecast);
  EXPECT_EQ(60, wind_v2->utc_min_forecast);
  EXPECT_EQ(0, wind_v2->duration);
  EXPECT_EQ(0, wind_v2->spare2);
}

}  // namespace
}  // namespace libais
