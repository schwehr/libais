// Test parsing 8:367:22.

#include <memory>

#include "gunit.h"
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

void Validate(
    const Ais8_367_22 *msg,
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
  ASSERT_EQ(8, msg->message_id);
  ASSERT_EQ(repeat_indicator, msg->repeat_indicator);
  ASSERT_EQ(mmsi, msg->mmsi);
  ASSERT_EQ(367, msg->dac);
  ASSERT_EQ(22, msg->fi);

  ASSERT_EQ(0, msg->spare);

  ASSERT_EQ(link_id, msg->link_id);
  ASSERT_EQ(notice_type, msg->notice_type);
  ASSERT_EQ(month, msg->month);
  ASSERT_EQ(day, msg->day);
  ASSERT_EQ(hour, msg->hour);
  ASSERT_EQ(minute, msg->minute);
  ASSERT_EQ(duration_minutes, msg->duration_minutes);
}


TEST(Ais8_367_22Test, DecodeSingleText) {
  // Invalid USCG test message with only a TEXT subpacket.
  // The parser allows it anyway and assumes that the caller will validate.
  // ,b2003669952,1413914824
  std::unique_ptr<Ais8_367_22> msg = Init(
      "!AIVDM,1,1,,A,803Ow2iKmPFJwP37P000bbHHsrPbJP000000,0*6E");
  Validate(msg.get(), 0, 3669771, 410, 127, 0, 0, 24, 60, 0);

  ASSERT_EQ(1, msg->sub_areas.size());

  ASSERT_EQ(AIS8_366_22_SHAPE_TEXT, msg->sub_areas[0]->getType());

  Ais8_367_22_Text *text = dynamic_cast<Ais8_367_22_Text *>(msg->sub_areas[0]);

  ASSERT_EQ("USCG-TEST@@@@@@", text->text);
  ASSERT_EQ(0, text->spare);
}

}  // namespace
}  // namespace libais
