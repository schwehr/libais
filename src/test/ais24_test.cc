// Test parsing message 24 - Class B Static Data Report.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais24> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais24> msg(new Ais24(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais24 *msg,
    const int repeat_indicator,
    const int mmsi,
    const int part_num,
    const string name,
    const int type_and_cargo,
    const string vendor_id,
    const string callsign,
    const int dim_a,
    const int dim_b,
    const int dim_c,
    const int dim_d,
    const int spare) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(24, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(part_num, msg->part_num);
  EXPECT_EQ(name, msg->name);
  EXPECT_EQ(type_and_cargo, msg->type_and_cargo);
  EXPECT_EQ(vendor_id, msg->vendor_id);
  EXPECT_EQ(callsign, msg->callsign);
  EXPECT_EQ(dim_a, msg->dim_a);
  EXPECT_EQ(dim_b, msg->dim_b);
  EXPECT_EQ(dim_c, msg->dim_c);
  EXPECT_EQ(dim_d, msg->dim_d);
  EXPECT_EQ(spare, msg->spare);
}

TEST(Ais24Test, DecodePartA) {
  std::unique_ptr<Ais24> msg = Init(
      "!AIVDM,1,1,,A,H44cj<0DdvlHhuB222222222220,2*46");

  Validate(
      msg.get(), 0, 273347120, 0, "EKO-FLOT            ",
      0, "", "", 0, 0, 0, 0, 0);
}

// Part A is supposed to be 160 bytes, but takes 168 too.
TEST(Ais24Test, DecodePartA168) {
  std::unique_ptr<Ais24> msg = Init(
      "!AIVDM,1,1,,A,H69LVS370a4d6222222222222200,0*29");

  Validate(
      msg.get(), 0, 412558988, 0, "10JQKA              ",
      0, "", "", 0, 0, 0, 0, 0);
}

TEST(Ais24Test, DecodePartB) {
  std::unique_ptr<Ais24> msg = Init(
      "!AIVDM,1,1,,A,H02IDPDm3?=1B00@9<?D00081110,0*6D");

  Validate(
      msg.get(), 0, 2512001, 1, "", 53, "COMAR@@", "PILOT@@", 1, 1, 1, 1, 0);
}

// Incorrectly using part B to store part A.
TEST(Ais24Test, DecodePartAinB1) {
  std::unique_ptr<Ais24> msg = Init(
      "!AIVDM,1,1,,A,H34l584400Hh;M1@VL000000000,2*07");

  // Suspiciously decodes like junk.  It would be good to know
  // why this message is being broadcast and what the actual
  // vessel name is supposed to be.  Is it bad AIS hardware?
  // Is the ship name really supposed to be "A"?
  Validate(
      msg.get(), 0, 206374176, 0, "A@@FLB7PTI`@@@@@@@@@",
      0, "", "", 0, 0, 0, 0, 0);
}

// Incorrectly using part B to store part A.
TEST(Ais24Test, DecodePartAinB2) {
  std::unique_ptr<Ais24> msg = Init(
      "!AIVDM,1,1,,A,H69oQAlh4V1QDr377C362222220,2*69");

  Validate(
      msg.get(), 0, 413000007, 0, "LAI XUN 11401       ",
      0, "", "", 0, 0, 0, 0, 0);
}

// Incorrectly using part B to store part A.
TEST(Ais24Test, DecodePartAinB3) {
  std::unique_ptr<Ais24> msg = Init(
      "!AIVDM,1,1,,B,H3tlF0TTT0000000004>M3Q<4h0,2*23");

  // Suspiciously decodes like junk.  Is this ship really supposed to
  // be named "II"?  Is there a bad embedded software on this Class B AIS?
  Validate(
      msg.get(), 0, 265098754, 0, "II@@@@@@@@@AC`P8SAL@",
      0, "", "", 0, 0, 0, 0, 0);
}

// Part C (2) is not defined as of ITU 1371-5.
TEST(Ais24Test, DecodePartC) {
  std::unique_ptr<Ais24> msg = Init(
      "!AIVDM,1,1,,A,H4eHhPLN000000038inPPP0P;330,0*4A");

  ASSERT_EQ(nullptr, msg);
}

// Part D (3) is not defined as of ITU 1371-5.
TEST(Ais24Test, DecodePartD) {
  std::unique_ptr<Ais24> msg = Init(
      "!AIVDM,1,1,,B,H3<D4q>0A8psB23B0tD4l000000,2*4C");

  ASSERT_EQ(nullptr, msg);
}

}  // namespace
}  // namespace libais
