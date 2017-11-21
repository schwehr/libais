// Test parsing message 26 - .

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

void Validate(
    const Ais26 *msg,
    const int repeat_indicator,
    const int mmsi,
    const bool use_app_id,
    const bool dest_mmsi_valid,
    const int dest_mmsi,
    const int dac,
    const int fi) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(26, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(use_app_id, msg->use_app_id);
  EXPECT_EQ(dest_mmsi_valid, msg->dest_mmsi_valid);
  EXPECT_EQ(dest_mmsi, msg->dest_mmsi);
  EXPECT_EQ(dac, msg->dac);
  EXPECT_EQ(fi, msg->fi);
}

TEST(Ais26Test, DecodeAnything) {
  // !AIVDM,2,1,2,B,JfgwlGvNwts9?wUfQswQ<gv9Ow7wCl?nwv0wOi=mwd?,0*03
  // !AIVDM,2,2,2,B,oW8uwNg3wNS3tV,5*71
  std::unique_ptr<Ais26> msg(new Ais26(
      "JfgwlGvNwts9?wUfQswQ<gv9Ow7wCl?nwv0wOi=mwd?oW8uwNg3wNS3tV", 5));

  Validate(
      msg.get(), 2, 989852767, true, true, 666891186, 319, 62);

  // TODO(schwehr): Validate commstate.
}

TEST(Ais26Test, b69511642_TooFewBits_Addressed_UseAppId) {
  // An addressed message with the use app id flag set, but too few bits
  // so it does not have the space required for the dac and fi.
  std::unique_ptr<Ais26> msg(new Ais26("JCwwwJwtbwwwww", 3));
  EXPECT_TRUE(msg->use_app_id);
  EXPECT_TRUE(msg->had_error());
}

}  // namespace
}  // namespace libais
