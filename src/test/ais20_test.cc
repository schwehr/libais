// Test parsing message 20 - Data link management.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais20> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais20> msg(new Ais20(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais20 *msg,
    const int repeat_indicator,
    const int mmsi,
    const int spare,
    const int offset_1,
    const int num_slots_1,
    const int timeout_1,
    const int incr_1,
    const bool group_valid_2,
    const int offset_2,
    const int num_slots_2,
    const int timeout_2,
    const int incr_2,
    const bool group_valid_3,
    const int offset_3,
    const int num_slots_3,
    const int timeout_3,
    const int incr_3,
    const bool group_valid_4,
    const int offset_4,
    const int num_slots_4,
    const int timeout_4,
    const int incr_4,
    const int spare2) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(20, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(spare, msg->spare);
  EXPECT_EQ(offset_1, msg->offset_1);
  EXPECT_EQ(num_slots_1, msg->num_slots_1);
  EXPECT_EQ(timeout_1, msg->timeout_1);
  EXPECT_EQ(incr_1, msg->incr_1);
  EXPECT_EQ(group_valid_2, msg->group_valid_2);
  EXPECT_EQ(offset_2, msg->offset_2);
  EXPECT_EQ(num_slots_2, msg->num_slots_2);
  EXPECT_EQ(timeout_2, msg->timeout_2);
  EXPECT_EQ(incr_2, msg->incr_2);
  EXPECT_EQ(group_valid_3, msg->group_valid_3);
  EXPECT_EQ(offset_3, msg->offset_3);
  EXPECT_EQ(num_slots_3, msg->num_slots_3);
  EXPECT_EQ(timeout_3, msg->timeout_3);
  EXPECT_EQ(incr_3, msg->incr_3);
  EXPECT_EQ(group_valid_4, msg->group_valid_4);
  EXPECT_EQ(offset_4, msg->offset_4);
  EXPECT_EQ(num_slots_4, msg->num_slots_4);
  EXPECT_EQ(timeout_4, msg->timeout_4);
  EXPECT_EQ(incr_4, msg->incr_4);
  EXPECT_EQ(spare2, msg->spare2);
}

TEST(Ais20Test, DecodeAnything) {
  std::unique_ptr<Ais20> msg = Init(
      "!SAVDM,1,1,6,B,Dh3OwjhflnfpLIF9HM1F9HMaF9H,2*3E");

  Validate(
      msg.get(), 3, 3669963, 0,
      749, 3, 3, 750,
      true, 454, 5, 3, 150,  // Slot 2.
      true, 464, 5, 3, 150,  // Slot 3.
      true, 474, 5, 3, 150,  // Slot 4.
      0);

  // TODO(schwehr): Handle GNSS payload.
}

}  // namespace
}  // namespace libais
