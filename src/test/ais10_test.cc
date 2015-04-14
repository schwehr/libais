// Test parsing message 10 ":" - UTC and date inquiry.

// TODO(schwehr): Decode these.  They might not be valid, but they were
// received in the wild.

// !AIVDM,1,1,,B,:T0Vd`,3*2A
// !AIVDM,1,1,,B,:1pwe20<m>FB,0*25
// !AIVDM,1,1,,B,:A6P?cNiukR`,0*6F
// !AIVDM,1,1,,B,:WHeWAEwGWgw,0*41
// !AIVDM,1,1,,B,:iCn44crw>GI,0*0D
// !AIVDM,1,1,,B,:lM>=N?JwNM?,0*4D
// !SAVDM,1,1,,B,:5@SW:19pedP,0*67
// !AIVDM,1,1,,B,:8MEJAmIknK;5cm5:ogsp,3*49
// !AIVDM,1,1,,A,:RFLT2OO>ITV@VUC4IRHpavl,5*3E
// !AIVDM,1,1,,A,:J`=1W5qKhACoa<7:hhk<k=wP,5*14
// !AIVDM,1,1,,B,:CFlQi3vqjM8;W98tPnSK`g<W9cqE;h<tP,1*12

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais10> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais10> msg(new Ais10(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais10 *msg,
    const int message_id,
    const int repeat_indicator,
    const int mmsi,
    const int spare,
    const int dest_mmsi,
    const int spare2) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(message_id, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(spare, msg->spare);
  EXPECT_EQ(dest_mmsi, msg->dest_mmsi);
  EXPECT_EQ(spare2, msg->spare2);
}

TEST(Ais10Test, DecodeAnything) {
  std::unique_ptr<Ais10> msg = Init("!AIVDM,1,1,,A,:5Ovc200B=5H,0*43");
  Validate(msg.get(), 10, 0, 369077000, 0, 1193046, 0);
}

}  // namespace
}  // namespace libais
