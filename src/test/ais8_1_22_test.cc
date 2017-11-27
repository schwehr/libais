// Test parsing 8:1:22.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais8_1_22> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais8_1_22> msg(new Ais8_1_22(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais8_1_22 *msg,
    const int repeat_indicator,
    const int mmsi,
    const int link_id,
    const int notice_type,
    const int month,
    const int day,
    const int hour,
    const int minute,
    const int duration_minutes) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(8, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(1, msg->dac);
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

// Tests decoding a Boston right whale alert message.
TEST(Ais8_1_22Test, CircleAndTextForMarineMammals) {
  std::unique_ptr<Ais8_1_22> msg = Init(
      "!AIVDM,1,1,0,B,803Ovrh0EPM0WB0h2l0MwJUi=6B4G9000aip8<2Bt2H"
      "q2Qhp,0*01,d-084,S1582,t091042.00,T42.19038981,r003669945,"
      "1332321042");
  Validate(msg.get(), 0, 3669739, 29, 1, 3, 20, 16, 6, 1440);

  EXPECT_EQ(2, msg->sub_areas.size());

  EXPECT_EQ(AIS8_1_22_SHAPE_CIRCLE, msg->sub_areas[0]->getType());
  EXPECT_EQ(AIS8_1_22_SHAPE_TEXT, msg->sub_areas[1]->getType());

  Ais8_1_22_Circle *circle =
      dynamic_cast<Ais8_1_22_Circle *>(msg->sub_areas[0]);

  EXPECT_FLOAT_EQ(-70.22429656982422, circle->position.lng_deg);
  EXPECT_FLOAT_EQ(42.105865478515625, circle->position.lat_deg);
  EXPECT_EQ(4, circle->precision);
  EXPECT_EQ(14810, circle->radius_m);
  EXPECT_EQ(0, circle->spare);

  Ais8_1_22_Text *text = dynamic_cast<Ais8_1_22_Text *>(msg->sub_areas[1]);

  EXPECT_STREQ("NOAA RW SGHTNG", text->text.c_str());
}

// Tests missing subareas.
TEST(Ais8_1_22Test, BadAreaNotice) {
  std::unique_ptr<Ais8_1_22> msg = Init(
      "!AIVDM,1,1,,A,803Ovrh0EPG0WB5p2l0L40,4*1E,d-085,S1593"
      ",t091042.00,T42.48370895,r003669945,1332321042");
  ASSERT_EQ(nullptr, msg);
}

// Tests missing subareas and incorrect pad.
TEST(Ais8_1_22Test, BadAreaNoticeAndWrongPad) {
  std::unique_ptr<Ais8_1_22> msg = Init(
      "!AIVDM,1,1,,A,803Ovrh0EPG0WB5p2l0L40,4*1E,d-085,S1593"
      ",t091042.00,T42.48370895,r003669945,1332321042");
  ASSERT_EQ(nullptr, msg);
}

TEST(Ais8_1_22Test, NoaaDMA) {
  // Testing NOAA DMAs
  // !AIVDM,2,1,1,A,803Ovrh0EPJ0Vvch00@=w52I9BK<00000VFHkP0>D>3,0*24
  // !AIVDM,2,2,1,A,;J005>?11PBGP4=1PPP,0*3F

  constexpr char payload[] =
    "803Ovrh0EPJ0Vvch00@=w52I9BK<00000VFHkP0>D>3;J005>?11PBGP4=1PPP";
  Ais8_1_22 msg(payload, 0);

  ASSERT_EQ(AIS_OK,  msg.get_error());
  EXPECT_EQ(3669739, msg.mmsi);
  EXPECT_STREQ("Caution Area: Marine mammals in area - reduce speed",
               ais8_1_22_notice_names[msg.notice_type]);
  EXPECT_EQ(26, msg.link_id);
  EXPECT_EQ(3, msg.month);
  EXPECT_EQ(15, msg.day);
  EXPECT_EQ(21, msg.hour);
  EXPECT_EQ(30, msg.minute);
  EXPECT_EQ(2, msg.duration_minutes);

  ASSERT_EQ(AIS8_1_22_SHAPE_CIRCLE, msg.sub_areas[0]->getType());
  Ais8_1_22_Circle* sub_area0 =
    dynamic_cast<Ais8_1_22_Circle *>(msg.sub_areas[0]);
  EXPECT_EQ(0, sub_area0->radius_m);
  EXPECT_DOUBLE_EQ(-70.408216666666661, sub_area0->position.lng_deg);
  EXPECT_DOUBLE_EQ(40.02495, sub_area0->position.lat_deg);

  ASSERT_EQ(AIS8_1_22_SHAPE_POLYGON, msg.sub_areas[1]->getType());
  Ais8_1_22_Polygon* sub_area1 =
    dynamic_cast<Ais8_1_22_Polygon *>(msg.sub_areas[1]);
  EXPECT_DOUBLE_EQ(103000.0, sub_area1->dists_m[0]);
  EXPECT_DOUBLE_EQ(114000.0, sub_area1->dists_m[1]);
  EXPECT_DOUBLE_EQ(101000.0, sub_area1->dists_m[2]);
  // TODO(rolker): 89.5?
  EXPECT_DOUBLE_EQ(179.0, sub_area1->angles[0]);
  EXPECT_DOUBLE_EQ(0.0, sub_area1->angles[1]);
  // TODO(rolker): 270?
  EXPECT_DOUBLE_EQ(540.0, sub_area1->angles[2]);

  ASSERT_EQ(AIS8_1_22_SHAPE_TEXT, msg.sub_areas[2]->getType());
  Ais8_1_22_Text* sub_area2 = dynamic_cast<Ais8_1_22_Text*>(msg.sub_areas[2]);
  EXPECT_EQ("NOAA RW DMA   ", sub_area2->text);
}

}  // namespace
}  // namespace libais
