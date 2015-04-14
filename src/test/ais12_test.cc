// Test parsing message 12 '<' - Addressed safety related text.

// TODO(schwehr): Find more test cases.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

// TODO(schwehr): Create an Init function that can handle multiple lines.

void Validate(
    const Ais12 *msg,
    const int message_id,
    const int repeat_indicator,
    const int mmsi,
    const int seq_num,
    const int dest_mmsi,
    const bool retransmitted,
    const int spare,
    const string &text) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());


  ASSERT_EQ(message_id, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(seq_num, msg->seq_num);
  EXPECT_EQ(dest_mmsi, msg->dest_mmsi);
  EXPECT_EQ(retransmitted, msg->retransmitted);
  EXPECT_EQ(spare, msg->spare);
  EXPECT_EQ(text, msg->text);
}

TEST(Ais12Test, DecodeAnything) {
  // clang-format off
  // !AIVDM,2,1,1,A,<02PeAPpIkF06B?=PB?31P3?>DB?<rP@<51C5P3?>D13DPB?31P3?>DB,0*13 NOLINT
  // !AIVDM,2,2,1,A,?<P?>PF86P381>>5<PoqP6?BP=1>41D?BIPB5@?BD@,4*66
  // clang-format on
  const string body("<02PeAPpIkF06B?=PB?31P3?>DB?<rP@<51C5P3?>D13DPB?31P3?>"
                    "DB?<P?>PF86P381>>5<PoqP6?BP=1>41D?BIPB5@?BD@");
  const int pad = 4;

  std::unique_ptr<Ais12> msg(new Ais12(body.c_str(), pad));

  Validate(
      msg.get(), 12, 0, 2633030, 0, 236572000, false, 0,
      "FROM ROCA CONTROL: PLEASE CONTACT ROCA CONTROL ON VHF CHANNEL 79 "
      "FOR MANDATORY REPORT");
}

}  // namespace
}  // namespace libais
