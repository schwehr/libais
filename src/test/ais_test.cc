// Test basic AIS infrastructure for AIVDM NMEA and AIS bit handling.

#include "ais.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::ElementsAre;

namespace libais {
namespace {

// Helper class that gives us access to the internal character table and thus
// let's us test it. Also allows us to easily create an AisBitset out of one
// of those internal character 6-bitsets.
class AisBitsetTester : public AisBitset {
 public:
  static AisBitset FromNmeaOrd(int index) {
    InitNmeaOrd();
    assert(index >= 0 && index < 128);
    AisBitsetTester result;
    result.initFromBitset6(AisBitset::nmea_ord_[index]);
    return result;
  }

 protected:
  void initFromBitset6(const bitset<6>& src) {
    for (int bit = 0; bit < 6; ++bit) {
      set(bit, src[bit]);
    }
    num_chars = 1;
    num_bits = 6;
  }
};

TEST(GetNthFieldTestEmpty, EmptyStrGetNthField) {
  const string line("");
  ASSERT_TRUE(GetNthField(line, 0, ",").empty());
  ASSERT_TRUE(GetNthField(line, 0, "yada").empty());
  ASSERT_TRUE(GetNthField(line, 1, ",").empty());
  ASSERT_TRUE(GetNthField(line, 1, "yada").empty());
}

// Test that GetNthField finds the correct field for single character fields.
TEST(GetNthFieldTest, FindCorrectFieldSimple) {
  const string line("a,b,c,d,e,f,g");

  ASSERT_EQ("a", GetNthField(line, 0, ","));
  ASSERT_EQ("b", GetNthField(line, 1, ","));
  ASSERT_EQ("c", GetNthField(line, 2, ","));
  ASSERT_EQ("d", GetNthField(line, 3, ","));
  ASSERT_EQ("e", GetNthField(line, 4, ","));
  ASSERT_EQ("f", GetNthField(line, 5, ","));
  ASSERT_EQ("g", GetNthField(line, 6, ","));
  ASSERT_EQ("", GetNthField(line, 7, ","));
  ASSERT_EQ("f", GetNthField(line, 5, ","));
  ASSERT_EQ("a,", GetNthField(line, 0, "b"));
  ASSERT_EQ(",g", GetNthField(line, 1, "f"));
  ASSERT_NE("a", GetNthField(line, 2, ","));
  ASSERT_NE("a", GetNthField(line, 1, "b"));
}

// Test some trickier strings.
TEST(GetNthFieldTest, FindMoreComplicated) {
  {
    const string line("");
    ASSERT_TRUE(GetNthField(line, 0, ",").empty());
    ASSERT_TRUE(GetNthField(line, 0, "yada").empty());
    ASSERT_TRUE(GetNthField(line, 1, ",").empty());
    ASSERT_TRUE(GetNthField(line, 1, "yada").empty());
  }

  {
    const string line("ab,cd,ef,gh");
    ASSERT_EQ("ab", GetNthField(line, 0, ","));
    ASSERT_EQ("cd", GetNthField(line, 1, ","));
    ASSERT_EQ("gh", GetNthField(line, 3, ","));
  }

  {
    const string line("ab,,,gh");
    ASSERT_EQ("ab", GetNthField(line, 0, ","));
    ASSERT_EQ("gh", GetNthField(line, 3, ","));
    ASSERT_EQ("", GetNthField(line, 2, ","));
  }

  {
    const string line(",yada,,");
    ASSERT_EQ("", GetNthField(line, 0, ","));
    ASSERT_EQ("yada", GetNthField(line, 1, ","));
    ASSERT_EQ("", GetNthField(line, 2, ","));
    ASSERT_EQ("", GetNthField(line, 3, ","));
  }
}

// Test that invalid indices return an empty string.
TEST(GetNthFieldTest, InvalidIndices) {
  const string line("a,b,c");
  ASSERT_TRUE(GetNthField(line, -1, ",").empty());
  ASSERT_TRUE(GetNthField(line, 4, ",").empty());
  ASSERT_TRUE(GetNthField(line, 13, ",").empty());
}

// Test cases where pad should return a valid number.
TEST(GetPadTest, SimpleFetch) {
  ASSERT_EQ(0, GetPad("!AIVDM,1,1,,A,100WhdhP0nJRdiFFHFvm??v00L12,0*13"));
  ASSERT_EQ(1, GetPad("!AIVDM,1,1,,A,g<0F8uc3551@0,1*56,raishub,1332549456"));
  ASSERT_EQ(2, GetPad("!AIVDM,2,2,4,A,000000000,2*20,raishub,1332547332"));
  ASSERT_EQ(4, GetPad("!AIVDM,1,1,,A,OtOf2G4D:9wNlqu?w@,4*6F,s21192"));
  ASSERT_EQ(5, GetPad("!AIVDM,1,1,,B,1?M37A5P089@wEN7KD1Khb5PhpL0,5*07"));
}

// Test pad failure cases.
TEST(GetPadTest, SurviveMalformed) {
  ASSERT_EQ(-1, GetPad(""));
  ASSERT_EQ(-1, GetPad("yada"));
  ASSERT_EQ(-1, GetPad(",,,,,,-1*13"));
  ASSERT_EQ(-1, GetPad(",,,,,,6*13"));
  ASSERT_EQ(-1, GetPad(",,,,,,16*13"));
}

// Can we reverse the bits in a bitset?
TEST(ReverseBitset6Test, ReverseWorks) {
  bitset<6> zero(0), bits1(0), bits2(0);
  ASSERT_EQ(bits1, AisBitset::Reverse(bits1));
  bits1[0] = 1;
  bits2[5] = 1;
  ASSERT_NE(bits1, AisBitset::Reverse(bits1));
  ASSERT_EQ(bits1, AisBitset::Reverse(bits2));
  bits1 = bits2 = zero;
  bits1[1] = 1;
  bits2[4] = 1;
  ASSERT_EQ(bits1, AisBitset::Reverse(bits2));
  bits1[2] = 1;
  bits2[3] = 1;
  ASSERT_EQ(bits1, AisBitset::Reverse(bits2));
}

// Test that the lookup table for NMEA AIS armoring returns the correct numbers.
TEST(BuildNmeaLookupTest, SpotCheckTable) {
  ASSERT_EQ(0, AisBitsetTester::FromNmeaOrd(48).ToUnsignedInt(0, 6));
  ASSERT_EQ(1, AisBitsetTester::FromNmeaOrd(49).ToUnsignedInt(0, 6));
  ASSERT_EQ(9, AisBitsetTester::FromNmeaOrd(57).ToUnsignedInt(0, 6));
  ASSERT_EQ(10, AisBitsetTester::FromNmeaOrd(58).ToUnsignedInt(0, 6));
  ASSERT_EQ(16, AisBitsetTester::FromNmeaOrd(64).ToUnsignedInt(0, 6));
  ASSERT_EQ(17, AisBitsetTester::FromNmeaOrd(65).ToUnsignedInt(0, 6));

  ASSERT_EQ(0, AisBitsetTester::FromNmeaOrd('0').ToUnsignedInt(0, 6));
  ASSERT_EQ(1, AisBitsetTester::FromNmeaOrd('1').ToUnsignedInt(0, 6));
  ASSERT_EQ(9, AisBitsetTester::FromNmeaOrd('9').ToUnsignedInt(0, 6));
  ASSERT_EQ(10, AisBitsetTester::FromNmeaOrd(58).ToUnsignedInt(0, 6));
  ASSERT_EQ(16, AisBitsetTester::FromNmeaOrd(64).ToUnsignedInt(0, 6));
  ASSERT_EQ(17, AisBitsetTester::FromNmeaOrd(65).ToUnsignedInt(0, 6));
  ASSERT_EQ(34, AisBitsetTester::FromNmeaOrd(
      static_cast<int>('R')).ToUnsignedInt(0, 6));
  ASSERT_EQ(34, AisBitsetTester::FromNmeaOrd('R').ToUnsignedInt(0, 6));
  ASSERT_EQ(39, AisBitsetTester::FromNmeaOrd('W').ToUnsignedInt(0, 6));
  ASSERT_EQ(39, AisBitsetTester::FromNmeaOrd(87).ToUnsignedInt(0, 6));
  // W-_ not used
  ASSERT_EQ(40, AisBitsetTester::FromNmeaOrd(96).ToUnsignedInt(0, 6));
  ASSERT_EQ(40, AisBitsetTester::FromNmeaOrd('`').ToUnsignedInt(0, 6));
  ASSERT_EQ(41, AisBitsetTester::FromNmeaOrd('a').ToUnsignedInt(0, 6));
  ASSERT_EQ(63, AisBitsetTester::FromNmeaOrd('w').ToUnsignedInt(0, 6));
  ASSERT_EQ(63, AisBitsetTester::FromNmeaOrd(119).ToUnsignedInt(0, 6));
  // x and above not used
}

// Tests that parsing an input initializes the members correctly.
TEST(BitsetPositionTest, TestParse) {
  AisBitset bitset;
  bitset.ParseNmeaPayload("123456", 0);
  ASSERT_EQ(36, bitset.GetNumBits());
  ASSERT_EQ(6, bitset.GetNumChars());
  ASSERT_EQ(36, bitset.GetRemaining());
}

// Tests that read operation result in the correct number of remaining bits.
TEST(BitsetPositionTest, TestRead) {
  AisBitset bitset;
  bitset.ParseNmeaPayload("123456", 0);

  bitset.ToInt(0, 6);
  ASSERT_EQ(30, bitset.GetRemaining());

  bitset.ToUnsignedInt(6, 8);
  ASSERT_EQ(22, bitset.GetRemaining());

  bitset.ToString(14, 12);
  ASSERT_EQ(10, bitset.GetRemaining());
}

// Tests that absolute and relative positioning within the bitset work.
TEST(BitsetPositionTest, TestSeek) {
  AisBitset bitset;
  bitset.ParseNmeaPayload("123456", 0);

  bitset.SeekTo(10);
  ASSERT_EQ(26, bitset.GetRemaining());

  bitset.SeekRelative(10);
  ASSERT_EQ(16, bitset.GetRemaining());

  bitset.SeekRelative(-4);
  ASSERT_EQ(20, bitset.GetRemaining());
}

}  // namespace
}  // namespace libais
