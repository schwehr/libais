// Test parsing message 15 - interrogation.

// TODO(schwehr): Decode these.  They might not be valid, but they were
// received in the wild.

// !AIVDM,1,1,,A,?WR1>7k9avqh,2*7A  - Undersized.
// !AIVDM,1,1,,A,?h3OwoiG`<q8D00,4*6E
// !AIVDM,1,1,,B,?8b=jTdUnktVmJL:,0*04
// !AIVDM,1,1,,B,?W4ekcOPOto8F;DP,2*0C
// !AIVDM,1,1,,B,?nvVNUjhG7cJo7vur0,4*09
// !AIVDM,1,1,,A,?>G3PvUawgnkbBgMvv0,2*6C
// !AIVDM,1,1,,B,?lGvAlVFCftW>GaG@eUi,0*3A
// !AIVDM,1,1,,A,?NcwsQvf7dnuQDM9m=vM<0,4*3B
// !AIVDM,1,1,,B,?7e4=glwoQvwR;o48vSOOk<,2*78
// !AIVDM,1,1,,A,?v;uNnv4urLSN=kKOrNVHmm?,0*5A
// !AIVDM,1,1,,A,?7VN8RKqwwTo=foWV7iwt?vLp,4*44
// !AIVDM,1,1,,A,?o999?O>k`W;WJladb?LowMba0,4*61
// !AIVDM,1,1,,A,?uUkOnmBwhJUwMM4<q7EK?tEdP0,2*63
// !AIVDM,1,1,,A,?CAgsD0024s9nQN:iati9Pth0D05,0*5D

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais15> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais15> msg(new Ais15(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais15 *msg,
    const int message_id,
    const int repeat_indicator,
    const int mmsi,
    const int spare,
    const int mmsi_1,
    const int msg_1_1,
    const int slot_offset_1_1,
    const int spare2,
    const int dest_msg_1_2,
    const int slot_offset_1_2,
    const int spare3,
    const int mmsi_2,
    const int msg_2,
    const int slot_offset_2,
    const int spare4) {
  ASSERT_NE(nullptr, msg);
  ASSERT_EQ(message_id, msg->message_id);
  ASSERT_EQ(repeat_indicator, msg->repeat_indicator);
  ASSERT_EQ(mmsi, msg->mmsi);
  ASSERT_EQ(spare, msg->spare);
  ASSERT_EQ(mmsi_1, msg->mmsi_1);
  ASSERT_EQ(msg_1_1, msg->msg_1_1);
  ASSERT_EQ(slot_offset_1_1, msg->slot_offset_1_1);
  ASSERT_EQ(spare2, msg->spare2);
  ASSERT_EQ(dest_msg_1_2, msg->dest_msg_1_2);
  ASSERT_EQ(slot_offset_1_2, msg->slot_offset_1_2);
  ASSERT_EQ(spare3, msg->spare3);
  ASSERT_EQ(mmsi_2, msg->mmsi_2);
  ASSERT_EQ(msg_2, msg->msg_2);
  ASSERT_EQ(slot_offset_2, msg->slot_offset_2);
  ASSERT_EQ(spare4, msg->spare4);
}


TEST(Ais15Test, DecodeAnything) {
  std::unique_ptr<Ais15> msg = Init(
      "!SAVDM,1,1,,B,?03OwnB0ACVlD00,2*59");

  Validate(
      msg.get(), 15, 0, 3669977, 0, 538005101, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

}  // namespace
}  // namespace libais
