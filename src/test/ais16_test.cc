// Test parsing message 16 - assigned mode command.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais16> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais16> msg(new Ais16(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais16 *msg,
    const int repeat_indicator,
    const int mmsi,
    const int spare,
    const int dest_mmsi_a,
    const int offset_a,
    const int inc_a,
    const int dest_mmsi_b,
    const int offset_b,
    const int inc_b,
    const int spare2) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(16, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(spare, msg->spare);
  EXPECT_EQ(dest_mmsi_a, msg->dest_mmsi_a);
  EXPECT_EQ(offset_a, msg->offset_a);
  EXPECT_EQ(inc_a, msg->inc_a);
  EXPECT_EQ(dest_mmsi_b, msg->dest_mmsi_b);
  EXPECT_EQ(offset_b, msg->offset_b);
  EXPECT_EQ(inc_b, msg->inc_b);
  EXPECT_EQ(spare2, msg->spare2);
}

TEST(Ais16Test, DecodeAnything) {
  // !SAVDO,1,1,,B,@03OwnQ9RgLP3h0000000000,0*32,b003669978,1426173689
  std::unique_ptr<Ais16> msg = Init(
      "!SAVDO,1,1,,B,@03OwnQ9RgLP3h0000000000,0*32");

  Validate(
      msg.get(), 0, 3669978, 0,
      308461000, 60, 0,  // Destination A.
      0, 0, 00,  // Destination B.
      -1);
}

TEST(Ais16Test, BitsLeftOver) {
  std::unique_ptr<Ais16> msg(new Ais16("@fffffhfffffffff", 4));
  EXPECT_TRUE(msg->had_error());
  EXPECT_EQ(AIS_ERR_BAD_BIT_COUNT, msg->get_error());
}

TEST(Ais16Test, InvalidButCommon168Bits) {
  // !AIVDM,1,1,,B,@bQBNdhP010Fh<LMb;:GLOvJP4@d,0*7F
  std::unique_ptr<Ais16> msg(new Ais16("@bQBNdhP010Fh<LMb;:GLOvJP4@d", 0));
  Validate(msg.get(), 2, 705994419, 0, 134218757, 2819, 113, 916638301, 3199,
           922, -1);
}

}  // namespace
}  // namespace libais
