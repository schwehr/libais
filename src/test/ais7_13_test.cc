// Test parsing messages 7 and 13.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais7_13> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais7_13> msg(new Ais7_13(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais7_13 *msg,
    const int message_id,
    const int repeat_indicator,
    const int mmsi,
    const int spare,
    const vector<int> dest_mmsi,
    const vector<int> seq_num) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(message_id, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(spare, msg->spare);
  EXPECT_EQ(dest_mmsi, msg->dest_mmsi);
  EXPECT_EQ(seq_num, msg->seq_num);
}

TEST(Ais713Test, Msg7Length1) {
  // Tests decoding a length 1 ack.
  std::unique_ptr<Ais7_13> msg = Init("!AIVDM,1,1,,A,7jvPoD@mMq;U,0*09");
  ASSERT_NE(nullptr, msg);
  const vector<int> dest_mmsi = {224257209};
  const vector<int> seq_num = {1};
  Validate(msg.get(), 7, 3, 199767889, 0, dest_mmsi, seq_num);
}

}  // namespace
}  // namespace libais
