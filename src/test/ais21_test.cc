// Test parsing message 21 - ATON Status.

#include <memory>

#include "gunit.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais21> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais21> msg(new Ais21(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais21 *msg,
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
  ASSERT_EQ(20, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(spare, msg->spare);
}

TEST(Ais21Test, DecodeAnything) {
  // std::unique_ptr<Ais21> msg = Init(
  //    "");
  std::unique_ptr<Ais21> msg(new Ais21(body.c_str(), pad));

  Validate(
      msg.get(), 3, 3669963, 0,
);

  // TODO(schwehr): Handle GNSS payload.
}

}  // namespace
}  // namespace libais
