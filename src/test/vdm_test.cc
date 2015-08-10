// Copyright 2015 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Test vdm.cc NMEA AIS parsing.
//
// TODO(schwehr): Add benchmarking back in.

#include "vdm.h"

#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "gmock/gmock.h"
#include "ais.h"

using std::unique_ptr;

namespace libais {
namespace {

#ifdef BENCHMARK
static void BM_Checksum(const int iters) {
  for (int i = 0; i < iters; i++) {
    CHECK_EQ(Checksum("NVXDR,G,142983,,WPTTTG"), 0x00);
    CHECK_EQ(Checksum("AIVDM,1,1,,B,15MuTpPP00INJMh@vq`00?vJ00S0,0"), 0x12);
    CHECK_EQ(
        Checksum(
            "UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU"
            "UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU"
            "UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU"
            "UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU"),
        0x00);
  }
}
BENCHMARK(BM_Checksum);
#endif  // BENCHMARK

TEST(ChecksumTest, TrySomeValidMessages) {
  // $NVXDR,G,142983,,WPTTTG*00
  EXPECT_EQ(0x00, Checksum("NVXDR,G,142983,,WPTTTG"));
  // $GPHDT,143.59,T*0F
  EXPECT_EQ(0x0F, Checksum("GPHDT,143.59,T"));
  // $INHDT,89.4,T*10
  EXPECT_EQ(0x10, Checksum("INHDT,89.4,T"));
  // $INROT,0,A*3F
  EXPECT_EQ(0x3F, Checksum("INROT,0,A"));
  // \n:121650,s:r17MHOP1,c:1425327399*1D\$ANZDA,201638.00,02,03,2015,00,00*77
  EXPECT_EQ(0x1D, Checksum("n:121650,s:r17MHOP1,c:1425327399"));
  // \g:2-2-223509,n:453068*26\$ARVSI,r08XSTL1,,201704.05846325,0152,-057,0*0B
  EXPECT_EQ(0x26, Checksum("g:2-2-223509,n:453068"));
}

TEST(ValidateChecksumTest, TrySomeMessages) {
  EXPECT_FALSE(ValidateChecksum(""));
  EXPECT_FALSE(ValidateChecksum("*"));
  EXPECT_FALSE(ValidateChecksum("UU*"));
  EXPECT_FALSE(ValidateChecksum("$NVXDR,G,142983,,WPTTTG*0"));
  EXPECT_FALSE(ValidateChecksum(" $NVXDR,G,142983,,WPTTTG*00"));

  EXPECT_TRUE(ValidateChecksum("$NVXDR,G,142983,,WPTTTG*00"));
  EXPECT_TRUE(ValidateChecksum("$INHDT,89.4,T*10"));
  EXPECT_TRUE(ValidateChecksum("\\n:121650,s:r17MHOP1,c:1425327399*1D"));
  EXPECT_TRUE(ValidateChecksum("!SAVDM,1,1,,B,K8VSqb9LdU28WP8P,0*7B"));
}

// TODO(schwehr): Test ToHex2.

#ifdef BENCHMARK
static void BM_NmeaSentenceCreate(const int iters) {
  // clang-format off
  const vector<string> lines = {
      "Ignore garbage input.", "!SAVDM,1,1,,B,K8VSqb9LdU28WP8P,0*7B",
      "!SAVDM,2,1,0,A,53U1J>02=S95=Hhp000lTpE9H60PDhDp0000001IIhV="
      "M6ue0ED22C0A1C42,0*38"  // NOLINT
  };
  // clang-format on

  for (int i = 0; i < iters; i++) {
    CHECK_EQ(nullptr, NmeaSentence::Create(lines[0], 1).get());
    CHECK_NE(nullptr, NmeaSentence::Create(lines[1], 2).get());
    CHECK_NE(nullptr, NmeaSentence::Create(lines[2], 3).get());
  }
}
BENCHMARK(BM_NmeaSentenceCreate);
#endif  // BENCHMARK

TEST(NmeaSentenceTest, ReturnNullptrOnJunk) {
  EXPECT_EQ(nullptr, NmeaSentence::Create("", 1));
  EXPECT_EQ(nullptr, NmeaSentence::Create("!AIVDM", 1));
  EXPECT_EQ(nullptr, NmeaSentence::Create(
                         "!AIVDM,1,1,,B,15MuTpPP00INJMh@vq`00?vJ00S0,0*1", 1));
  // TODO(schwehr): Make all the checksums match.
  EXPECT_EQ(nullptr, NmeaSentence::Create(
                         "AIVDM,1,1,,B,15MuTpPP00INJMh@vq`00?vJ00S0,0*23", 1));
  EXPECT_EQ(nullptr, NmeaSentence::Create(
                         "!AIVDM,,1,,B,15MuTpPP00INJMh@vq`00?vJ00S0,0*23", 1));
  EXPECT_EQ(nullptr, NmeaSentence::Create(
                         "!AIVDM,1,,,B,15MuTpPP00INJMh@vq`00?vJ00S0,0*23", 1));
  EXPECT_EQ(nullptr,
            NmeaSentence::Create(
                "!AIVDM,1,1,A,B,15MuTpPP00INJMh@vq`00?vJ00S0,0*53", 1));
  EXPECT_EQ(nullptr, NmeaSentence::Create(
                         "!AIVDM,1,1,,1,15MuTpPP00INJMh@vq`00?vJ00S0,0*61", 1));
  // Sentence total > sentence number.
  EXPECT_EQ(nullptr, NmeaSentence::Create(
                         "!AIVDM,1,2,,B,15MuTpPP00INJMh@vq`00?vJ00S0,0*11", 1));
  // Must start with !.
  EXPECT_EQ(nullptr, NmeaSentence::Create(
                         "$AIVDM,1,1,,B,15MuTpPP00INJMh@vq`00?vJ00S0,0*12", 1));
  // Missing fill_bits.
  EXPECT_EQ(nullptr, NmeaSentence::Create(
                         "!AIVDM,1,1,,B,15MuTpPP00INJMh@vq`00?vJ00S0,*12", 1));
  // fill_bits > 5.
  EXPECT_EQ(nullptr, NmeaSentence::Create(
                         "!AIVDM,1,1,,B,15MuTpPP00INJMh@vq`00?vJ00S0,6*14", 1));
  // Bad checksum.
  EXPECT_EQ(nullptr, NmeaSentence::Create(
                         "!AIVDM,1,1,,B,15MuTpPP00INJMh@vq`00?vJ00S0,0*13", 1));
  // Extra metadata not allowed.
  EXPECT_EQ(nullptr, NmeaSentence::Create(
                         "!AIVDM,1,1,,B,15MuTpPP00INJMh@vq`00?vJ00S0,0*12,"
                         "b003669950,1380725114",
                         1));
  // Extra number on the checksum.
  EXPECT_EQ(nullptr,
            NmeaSentence::Create(
                "!AIVDM,1,1,,B,15MuTpPP00INJMh@vq`00?vJ00S0,0*123", 1));
}

TEST(NmeaSentenceTest, SingleLines) {
  // Message 27 - Low bit count position report.
  int64_t line_number = 12;
  string line("!SAVDM,1,1,,B,K8VSqb9LdU28WP8P,0*7B");
  auto sentence = NmeaSentence::Create(line, line_number);
  ASSERT_NE(nullptr, sentence);
  EXPECT_EQ("SA", sentence->talker());
  EXPECT_EQ("VDM", sentence->sentence_type());
  EXPECT_EQ(1, sentence->sentence_total());
  EXPECT_EQ(1, sentence->sentence_number());
  EXPECT_EQ(-1, sentence->sequence_number());
  EXPECT_EQ('B', sentence->channel());
  EXPECT_EQ("K8VSqb9LdU28WP8P", sentence->body());
  EXPECT_EQ(0, sentence->fill_bits());
  EXPECT_EQ(line_number, sentence->line_number());
  ASSERT_EQ(line, sentence->ToString());
  // TODO(schwehr): Add MD5.
  // ASSERT_EQ("6e1a4872825054e91ee7cfcfb9cc87e0", sentence->ToMd5Digest());

  line_number = 42;
  line =
      "!SAVDM,2,1,0,A,53U1J>02=S95=Hhp000lTpE9H60PDhDp0000001IIhV=M6ue0ED2"
      "2C0A1C42,0*38";
  sentence = NmeaSentence::Create(line, line_number);
  ASSERT_NE(nullptr, sentence);
  EXPECT_EQ("SA", sentence->talker());
  EXPECT_EQ("VDM", sentence->sentence_type());
  EXPECT_EQ(2, sentence->sentence_total());
  EXPECT_EQ(1, sentence->sentence_number());
  EXPECT_EQ(0, sentence->sequence_number());
  EXPECT_EQ('A', sentence->channel());
  EXPECT_EQ("53U1J>02=S95=Hhp000lTpE9H60PDhDp0000001IIhV=M6ue0ED22C0A1C42",
            sentence->body());
  EXPECT_EQ(0, sentence->fill_bits());
  EXPECT_EQ(line_number, sentence->line_number());
  ASSERT_EQ(line, sentence->ToString());
  // TODO(schwehr): Add MD5.
  // ASSERT_EQ("f61e1ab15c12865196377a8ea4071660", sentence->ToMd5Digest());

  line_number = 1234567890123;
  line = "!BSVDM,2,2,5,A,580CPj88880,2*7C";
  sentence = NmeaSentence::Create(line, line_number);
  ASSERT_NE(nullptr, sentence);
  EXPECT_EQ("BS", sentence->talker());
  EXPECT_EQ("VDM", sentence->sentence_type());
  EXPECT_EQ(2, sentence->sentence_total());
  EXPECT_EQ(2, sentence->sentence_number());
  EXPECT_EQ(5, sentence->sequence_number());
  EXPECT_EQ('A', sentence->channel());
  EXPECT_EQ("580CPj88880", sentence->body());
  EXPECT_EQ(2, sentence->fill_bits());
  EXPECT_EQ(line_number, sentence->line_number());
  EXPECT_EQ(line, sentence->ToString());
  // TODO(schwehr): Add MD5.
  // ASSERT_EQ("076754977d78035f5a10ec1bce063a87", sentence->ToMd5Digest());

  line_number = 3;
  line = "!AIVDM,2,2,8,B,v@,5*2E";
  sentence = NmeaSentence::Create(line, line_number);
  ASSERT_NE(nullptr, sentence);
  EXPECT_EQ("AI", sentence->talker());
  EXPECT_EQ("VDM", sentence->sentence_type());
  EXPECT_EQ(2, sentence->sentence_total());
  EXPECT_EQ(2, sentence->sentence_number());
  EXPECT_EQ(8, sentence->sequence_number());
  EXPECT_EQ('B', sentence->channel());
  EXPECT_EQ("v@", sentence->body());
  EXPECT_EQ(5, sentence->fill_bits());
  EXPECT_EQ(line_number, sentence->line_number());
  EXPECT_EQ(line, sentence->ToString());
  // TODO(schwehr): Add MD5.
  // ASSERT_EQ("15d2f7a2d21883457ccfc28734a21da3", sentence->ToMd5Digest());
}

#ifdef BENCHMARK
static void BM_NmeaSentenceMerge(const int iters) {
  // clang-format off
  const vector<string> lines = {
      "!SAVDM,2,1,1,A,54a=3b027kft?HISV20@thF0<u=@618T<6222216A0b<?4wk0BAm@F@"
      "DEBC8,0*17",  // NOLINT
      "!SAVDM,2,2,1,A,88888888880,2*3F"};
  // clang-format on
  for (int i = 0; i < iters; i++) {
    vector<unique_ptr<NmeaSentence>> priors;
    priors.emplace_back(NmeaSentence::Create(lines[0], 1));
    auto sentence2 = NmeaSentence::Create(lines[1], 2);
    auto sentence = sentence2->Merge(priors);
    EXPECT_NE(nullptr, sentence);
  }
}
BENCHMARK(BM_NmeaSentenceMerge);
#endif  // BENCHMARK

TEST(NmeaSentenceTest, Merge) {
  // clang-format off
  const vector<string> lines = {
      "!SAVDM,2,1,1,A,54a=3b027kft?HISV20@thF0<u=@618T<6222216A0b<?4wk0BAm@F@"
      "DEBC8,0*17",  // NOLINT
      "!SAVDM,2,2,1,A,88888888880,2*3F"};
  // clang-format on

  vector<unique_ptr<NmeaSentence>> priors;
  priors.emplace_back(NmeaSentence::Create(lines[0], 1));
  auto sentence2 = NmeaSentence::Create(lines[1], 2);
  auto sentence = sentence2->Merge(priors);
  EXPECT_TRUE(sentence);
  EXPECT_EQ(
      "54a=3b027kft?HISV20@thF0<u=@618T<6222216A0b<?4wk0BAm@F@"
      "DEBC888888888880",
      sentence->body());
  EXPECT_EQ(1, sentence->sentence_number());
  EXPECT_EQ(1, sentence->sentence_total());
  EXPECT_EQ(1, sentence->sentence_number());
  EXPECT_EQ('A', sentence->channel());
}

TEST(NmeaSentenceTest, VerifyInSameMessage) {
  NmeaSentence s1of3("AI", "VDM", 3, 1, 4, 'A', "body1", 0, 1);
  NmeaSentence s2of3("AI", "VDM", 3, 2, 4, 'A', "body2", 0, 2);
  NmeaSentence s3of3("AI", "VDM", 3, 3, 4, 'A', "body3", 0, 3);

  EXPECT_TRUE(s1of3.VerifyInSameMessage(s2of3));
  EXPECT_TRUE(s1of3.VerifyInSameMessage(s3of3));
  EXPECT_TRUE(s2of3.VerifyInSameMessage(s1of3));
  EXPECT_TRUE(s2of3.VerifyInSameMessage(s3of3));
  EXPECT_TRUE(s3of3.VerifyInSameMessage(s1of3));
  EXPECT_TRUE(s3of3.VerifyInSameMessage(s2of3));

  EXPECT_FALSE(s1of3.VerifyInSameMessage(
      NmeaSentence("BS", "VDM", 3, 2, 4, 'A', "body1", 0, 4)));
  EXPECT_FALSE(s1of3.VerifyInSameMessage(
      NmeaSentence("AI", "VDO", 3, 2, 4, 'A', "body1", 0, 5)));
  EXPECT_FALSE(s1of3.VerifyInSameMessage(
      NmeaSentence("AI", "VDM", 4, 2, 4, 'A', "body1", 0, 6)));
  EXPECT_FALSE(s1of3.VerifyInSameMessage(
      NmeaSentence("AI", "VDM", 3, 2, 5, 'A', "body1", 0, 7)));
  EXPECT_FALSE(s1of3.VerifyInSameMessage(
      NmeaSentence("AI", "VDM", 3, 2, 4, 'B', "body1", 0, 8)));
}

#ifdef BENCHMARK
static void BM_VdmStream(const int iters) {
  const vector<string> single_line_messages = {
      "!SAVDM,1,1,6,A,15N4uK0P00r<rW:BFp;JJgv`25k`,0*49",
      "!SAVDM,1,1,,A,29NS6m1000qE>9f@s=BES4M40@ET,0*53",
      "!AIVDM,1,1,,A,4030p:Autt01Dn`erRO<Wno00@<I,0*41",
      "!BSVDM,1,1,,A,B52HIjh00=ksdj6l448=wwQ5WP06,0*68",
      "!SAVDM,1,1,,B,K8VSqb9LdU28WP8d,0*4F"};
  // clang-format off
  const vector<string> two_line_message = {
      "!SAVDM,2,1,1,A,54a=3b027kft?HISV20@thF0<u=@618T<6222216A0b<?4wk0BAm@F@"
      "DEBC8,0*17",  // NOLINT
      "!SAVDM,2,2,1,A,88888888880,2*3F"};
  // clang-format on
  VdmStream stream;

  for (int i = 0; i < iters; i++) {
    for (const string &line : single_line_messages) {
      EXPECT_TRUE(stream.AddLine(line));
      ASSERT_NE(nullptr, stream.PopOldestMessage().get());
    }

    for (const string &line : two_line_message) {
      EXPECT_TRUE(stream.AddLine(line));
    }
    ASSERT_NE(nullptr, stream.PopOldestMessage().get());
  }
}
BENCHMARK(BM_VdmStream);
#endif  // BENCHMARK

class VdmTest : public ::testing::Test {
 protected:
  VdmStream stream_;
};

TEST_F(VdmTest, IgnoreNonAisMessages) {
  // None of these are complete and valid VDM messages.
  const vector<string> lines = {
      "",
      "junk",
      "# comment",
      "$ANZDA,000258.00,03,04,2014,00,00*71",
      "SAVDM,1,1,6,A,15N4uK0P00r<rW:BFp;JJgv`25k`,0*49",
      "!SAVDM,1,1,6,A,15N4uK0P00r<rW:BFp;JJgv`25k`,0"};

  ASSERT_EQ(nullptr, stream_.PopOldestMessage());

  for (const string &line : lines) {
    EXPECT_FALSE(stream_.AddLine(line));
    EXPECT_EQ(nullptr, stream_.PopOldestMessage());
  }
}

TEST_F(VdmTest, SingleLineAisMessage) {
  ASSERT_EQ(nullptr, stream_.PopOldestMessage());
  EXPECT_TRUE(
      stream_.AddLine("!SAVDM,1,1,6,A,15N4uK0P00r<rW:BFp;JJgv`25k`,0*49"));
  auto ais_msg = stream_.PopOldestMessage();
  ASSERT_NE(nullptr, ais_msg);
  ASSERT_EQ(1, ais_msg->message_id);
  auto msg = reinterpret_cast<libais::Ais1_2_3 *>(ais_msg.get());
  EXPECT_EQ(367082860, msg->mmsi);
  EXPECT_EQ(1, msg->slot_timeout);
  EXPECT_EQ(20, msg->timestamp);
  EXPECT_EQ(511, msg->true_heading);
}

TEST_F(VdmTest, QueueClean) {
  ASSERT_EQ(nullptr, stream_.PopOldestMessage());
  EXPECT_TRUE(
      stream_.AddLine("!SAVDM,1,1,6,A,15N4uK0P00r<rW:BFp;JJgv`25k`,0*49"));
  EXPECT_TRUE(
      stream_.AddLine("!SAVDM,1,1,,A,29NS6m1000qE>9f@s=BES4M40@ET,0*53"));
  // Queue left with unconsumed messages at destruction.
  // Should not leak here.
}

TEST_F(VdmTest, MultipleSingleLineAisMessages) {
  const vector<string> lines = {
      "!SAVDM,1,1,6,A,15N4uK0P00r<rW:BFp;JJgv`25k`,0*49",
      "!SAVDM,1,1,,A,29NS6m1000qE>9f@s=BES4M40@ET,0*53",
      "!BSVDM,1,1,,B,36So=l5000o?uF0K>pnpV0Nf0000,0*57",
      "!AIVDM,1,1,,A,4030p:Autt01Dn`erRO<Wno00@<I,0*41",
      "!BSVDM,1,1,,A,B52HIjh00=ksdj6l448=wwQ5WP06,0*68",
      "!SAVDM,1,1,,B,K8VSqb9LdU28WP8d,0*4F"};
  ASSERT_EQ(nullptr, stream_.PopOldestMessage());
  for (const string &line : lines) {
    EXPECT_TRUE(stream_.AddLine(line));
  }

  auto ais_msg = stream_.PopOldestMessage();
  ASSERT_NE(nullptr, ais_msg);
  ASSERT_EQ(1, ais_msg->message_id);
  auto msg1_2_3 = reinterpret_cast<libais::Ais1_2_3 *>(ais_msg.get());
  EXPECT_EQ(367082860, msg1_2_3->mmsi);
  EXPECT_EQ(20, msg1_2_3->timestamp);
  EXPECT_EQ(511, msg1_2_3->true_heading);
  msg1_2_3 = nullptr;

  ais_msg = stream_.PopOldestMessage();
  ASSERT_NE(nullptr, ais_msg);
  ASSERT_EQ(2, ais_msg->message_id);
  msg1_2_3 = reinterpret_cast<libais::Ais1_2_3 *>(ais_msg.get());
  EXPECT_EQ(636012244, msg1_2_3->mmsi);
  EXPECT_EQ(34, msg1_2_3->timestamp);
  EXPECT_EQ(142, msg1_2_3->true_heading);
  msg1_2_3 = nullptr;

  ais_msg = stream_.PopOldestMessage();
  ASSERT_NE(nullptr, ais_msg);
  ASSERT_EQ(3, ais_msg->message_id);
  msg1_2_3 = reinterpret_cast<libais::Ais1_2_3 *>(ais_msg.get());
  EXPECT_EQ(440258000, msg1_2_3->mmsi);
  EXPECT_EQ(23, msg1_2_3->timestamp);
  EXPECT_EQ(15, msg1_2_3->true_heading);
  msg1_2_3 = nullptr;

  ais_msg = stream_.PopOldestMessage();
  ASSERT_NE(nullptr, ais_msg);
  ASSERT_EQ(4, ais_msg->message_id);

  ais_msg = stream_.PopOldestMessage();
  ASSERT_NE(nullptr, ais_msg);
  ASSERT_EQ(18, ais_msg->message_id);

  ais_msg = stream_.PopOldestMessage();
  ASSERT_NE(nullptr, ais_msg);
  ASSERT_EQ(27, ais_msg->message_id);
}

string RightStrip(const string &src) {
  size_t pos = src.find_last_not_of(" \n\r\t");
  if (string::npos == pos) {
    return src;
  }
  return src.substr(0, pos + 1);
}

TEST_F(VdmTest, TwoLineAisMessage) {
  // clang-format off
  const vector<string> lines = {
      "!SAVDM,2,1,1,A,54a=3b027kft?HISV20@thF0<u=@618T<6222216A0b<?4wk0BAm@F@"
      "DEBC8,0*17",  // NOLINT
      "!SAVDM,2,2,1,A,88888888880,2*3F"};
  // clang-format on

  ASSERT_EQ(nullptr, stream_.PopOldestMessage());
  EXPECT_TRUE(stream_.AddLine(lines[0]));
  ASSERT_EQ(nullptr, stream_.PopOldestMessage());
  EXPECT_TRUE(stream_.AddLine(lines[1]));

  auto ais_msg = stream_.PopOldestMessage();
  ASSERT_NE(nullptr, ais_msg);
  ASSERT_EQ(5, ais_msg->message_id);
  auto msg5 = reinterpret_cast<libais::Ais5 *>(ais_msg.get());
  EXPECT_EQ(311641000, msg5->mmsi);
  EXPECT_EQ(8900335, msg5->imo_num);
  msg5->callsign = RightStrip(msg5->callsign);
  EXPECT_EQ("C6FX9", msg5->callsign);
  msg5->name = RightStrip(msg5->name);
  EXPECT_EQ("DOLE COSTA RICA", msg5->name);
  EXPECT_EQ(70, msg5->type_and_cargo);
}

TEST_F(VdmTest, ThreeLineAisMessage) {
  // clang-format off
  const vector<string> lines = {
      "!AIVDM,3,1,4,A,81mg=5@0EP:4R40807P>0<D1>MNt00000f>FNVfnw7>6>FNU=?"
      "B5PD5HDPD8,0*26",  // NOLINT
      "!AIVDM,3,2,4,A,1Dd2J09jL08JArJH5P=E<D9@<5P<9>0`bMl42Q0d2Pc2T59CPCE@@?"
      "C54PD?,0*60",  // NOLINT
      "!AIVDM,3,3,4,A,d0@d0IqhH:Pah:U54PD?75D85Bf00,0*03"};
  // clang-format off

  for (const string &line : lines) {
    ASSERT_TRUE(stream_.AddLine(line));
  }
  auto ais_msg = stream_.PopOldestMessage();
  ASSERT_NE(nullptr, ais_msg);
  ASSERT_EQ(8, ais_msg->message_id);
}

TEST_F(VdmTest, OverRangeSequence) {
  // The sequence number should be [0..9].  Reject others.
  stream_.AddLine("!BSVDM,1,1,10,B,36So=l5000o?uF0K>pnpV0Nf0000,0*56");
  auto ais_msg = stream_.PopOldestMessage();
  ASSERT_EQ(nullptr, ais_msg);
}

}  // namespace
}  // namespace libais
