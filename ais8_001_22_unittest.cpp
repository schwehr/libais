// Since 29-Mar-2011 Kurt Schwehr
// Google Test unit testing (started with 1.6.0rc) for the Area Notice
// Based on messages from ais_areanotice_py

#include <gtest/gtest.h>
#include <string>

#include "ais.h"
#include "ais8_001_22.h"

using namespace std;

TEST(EmptyTest, Empty) {
    // TODO(schwehr): test the empty string case.  What should it do?
}

TEST(HelperTest, Helper) {
    const string msg_str = "!AIVDM,1,1,,A,81mg=5@0EP:0>H0007P>0<D1<qp400000,0*1D";
    const string body(nth_field(msg_str, 5, ','));
    ASSERT_STREQ("81mg=5@0EP:0>H0007P>0<D1<qp400000", body.c_str());
}

// This is really not working right.  What is going on?
TEST(PointTest, Point) {
    build_nmea_lookup();

    // AreaNotice: type=0  start=2011-07-06 00:00:00  duration=60 m  link_id=10  sub-areas: 1
    const string msg_str = "!AIVDM,1,1,,A,81mg=5@0EP:0>H0007P>0<D1<qp400000,0*1D";
    const string body(nth_field(msg_str, 5));
    Ais8_001_22 msg(body.c_str(), 0);
    ASSERT_EQ(AIS_OK, msg.get_error());
    EXPECT_EQ(msg.message_id, 8);
    EXPECT_EQ(msg.repeat_indicator, 0);
    EXPECT_EQ(msg.mmsi, 123456789);
    EXPECT_EQ(msg.spare, 0);
    EXPECT_EQ(msg.dac, 1);
    EXPECT_EQ(msg.fi, 22);
}
