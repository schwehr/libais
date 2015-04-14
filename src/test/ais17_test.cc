// Test parsing message 17 - GNSS boradcast.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais17> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais17> msg(new Ais17(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais17 *msg,
    const int repeat_indicator,
    const int mmsi,
    const int spare,
    const float x,
    const float y,
    const int spare2,
    const int gnss_type,
    const int z_cnt,
    const int station,
    const int seq,
    const int health) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(17, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(spare, msg->spare);
  EXPECT_NEAR(x, msg->position.lng_deg, 0.0001);
  EXPECT_NEAR(y, msg->position.lat_deg, 0.0001);
  EXPECT_EQ(spare2, msg->spare2);
  EXPECT_EQ(gnss_type, msg->gnss_type);
  EXPECT_EQ(z_cnt, msg->z_cnt);
  EXPECT_EQ(station, msg->station);
  EXPECT_EQ(seq, msg->seq);
  EXPECT_EQ(health, msg->health);
}

TEST(Ais17Test, DecodeAnything) {
  // !SAVDO,1,1,,B,@03OwnQ9RgLP3h0000000000,0*32,b003669978,1426173689
  std::unique_ptr<Ais17> msg = Init(
      "!AIVDM,1,1,,A,A6WWW6gP00a3PDlEKLrarOwUr8Mg,0*03");

  Validate(
      msg.get(),
      0, 444196634, 3, -54.6133, 35.0333, 1, 19, 3513, 277, 6, 1);

  // TODO(schwehr): Handle GNSS payload.
}

}  // namespace
}  // namespace libais
