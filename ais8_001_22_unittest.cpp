// Since 29-Mar-2011 Kurt Schwehr
// Google Test unit testing (started with 1.6.0rc) for the Area Notice
// Based on messages from ais_areanotice_py

#include <gtest/gtest.h>
#include "ais.h"
#include "ais8_001_22.h"

#include <string>

using namespace std;

//const size_t find_nth(const string &str, const size_t n, const char c);
//const string nth_field(const string &str, const size_t n, const char c);

#if 0
const size_t find_nth(const string &str, const size_t n, const char c) {
    size_t pos;
    size_t count;
    for (pos=0, count=0; count < n and pos != string::npos; count+=1) {
        if (pos>0) pos += 1; // Skip past the current char that matched
        pos = str.find(c, pos);
        //cout << "p: " << pos << " " << count << endl;
    } 
    return pos;
}
#endif

#if 0
// equivalent to the python: my_string.split(c)[n]
const string nth_field(const string &str, const size_t n, const char c) {
    // FIX: handle the off the end case better
    size_t pos;
    size_t count;
    for (pos=0, count=0; count < n and pos != string::npos; count+=1) {
        if (pos>0) pos += 1; // Skip past the current char that matched
        pos = str.find(c, pos);
    } 
    if (string::npos == pos) return string("");
   
    const size_t start = pos;
    const size_t end = str.find(c, pos+1);
    if (string::npos == end) return str.substr(start);
    return str.substr(start+1, end-start-1);
}
#endif

TEST(EmptyTest, Empty) {
    // FIX: test the empty string case.  What should it do?
}

TEST(HelperTest, Helper) {
    // Does nth_field work right?
    const string msg_str = "!AIVDM,1,1,,A,81mg=5@0EP:0>H0007P>0<D1<qp400000,0*1D";
    const string body(nth_field(msg_str,5,','));
    //cout << "body: " << body << endl;
    ASSERT_STREQ("81mg=5@0EP:0>H0007P>0<D1<qp400000", body.c_str());
}

// This is really not working right.  What is going on?
TEST(PointTest, Point) {
    build_nmea_lookup();

    // AreaNotice: type=0  start=2011-07-06 00:00:00  duration=60 m  link_id=10  sub-areas: 1
    const string msg_str = "!AIVDM,1,1,,A,81mg=5@0EP:0>H0007P>0<D1<qp400000,0*1D";
    const string body(nth_field(msg_str,5,','));
    //cout << "body: " << body << endl;
    Ais8_001_22 msg(body.c_str());
    ASSERT_EQ(AIS_OK, msg.get_error());
    //cout << "msg8_1_22: " << endl;
    //msg.print();
    //cout << "mid:" << msg.message_id << endl;
    EXPECT_EQ(msg.message_id,8);
    EXPECT_EQ(msg.repeat_indicator,0);
    EXPECT_EQ(msg.mmsi,123456789);
    EXPECT_EQ(msg.spare,0);
    EXPECT_EQ(msg.dac,1);
    EXPECT_EQ(msg.fi,22);
} // PointTest
