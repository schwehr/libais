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

TEST(ais8dac001fi22, Empty) {
    // FIX: test the empty string case.  What should it do?
}

TEST(ais8dac001fi22, Helper) {
    // Does nth_field work right?
    const string msg_str = "!AIVDM,1,1,,A,81mg=5@0EP:0>H0007P>0<D1<qp400000,0*1D";
    const string body(nth_field(msg_str,5,','));
    //cout << "body: " << body << endl;
    ASSERT_STREQ("81mg=5@0EP:0>H0007P>0<D1<qp400000", body.c_str());
}

// This is really not working right.  What is going on?
TEST(ais8dac001fi22, Point) {
    build_nmea_lookup();

    // AreaNotice: type=0  start=2011-07-06 00:00:00  duration=60 m  link_id=10  sub-areas: 1
    const string msg_str = "!AIVDM,1,1,,A,81mg=5@0EP:0>H0007P>0<D1<qp400000,0*1D";
    const string body(nth_field(msg_str,5,','));
    //cout << "body: " << body << endl;
    Ais8_001_22 msg(body.c_str(),0);
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

TEST(ais8dac001fi22, Polygon) {
    {
        // !AIVDM,2,1,2,A,81mg=5@0EPO0VVbh00P=t<Ra<9;400000TFKVP1vL>>,0*74
        // !AIVDM,2,2,2,A,?J000,3*52
        
        const char * payload = "81mg=5@0EPO0VVbh00P=t<Ra<9;400000TFKVP1vL>>?J000";
        Ais8_001_22 msg(payload,0);
        //msg.print();
        
        ASSERT_EQ(AIS_OK, msg.get_error());
        EXPECT_EQ(msg.sub_areas.size(),2);
        
        const char * payload2 = "81mg=5@0EPd0VV`800@>2EKI>@uT00000TFNWP1Od>J";
        Ais8_001_22 msg2(payload2,0);
        //msg2.print();
        
        ASSERT_EQ(AIS_OK, msg2.get_error());
        
        
        // empty polygon
        const char * payload3 = "81mg=5@0EPD0Vn3PJip1TeD1TeD400000P0000000001J000";
        Ais8_001_22 msg3(payload3,0);
        //msg3.print();
        
        ASSERT_EQ(AIS_OK, msg3.get_error());
    }
}

TEST(ais8dac001fi22, NoaaDMA) {
    {
        
        // Testing NOAA DMAs
        // !AIVDM,2,1,1,A,803Ovrh0EPJ0Vvch00@=w52I9BK<00000VFHkP0>D>3,0*24
        // !AIVDM,2,2,1,A,;J005>?11PBGP4=1PPP,0*3F
        
        const char * payload4 = "803Ovrh0EPJ0Vvch00@=w52I9BK<00000VFHkP0>D>3;J005>?11PBGP4=1PPP";
        Ais8_001_22 msg4(payload4,0);
        
        ASSERT_EQ(AIS_OK, msg4.get_error());
        ASSERT_EQ(3669739, msg4.mmsi);
        ASSERT_STREQ("Caution Area: Marine mammals in area - reduce speed",ais8_001_22_notice_names[msg4.notice_type]);
        ASSERT_EQ(26,msg4.link_id);
        ASSERT_EQ(3,msg4.month);
        ASSERT_EQ(15,msg4.day);
        ASSERT_EQ(21,msg4.hour);
        ASSERT_EQ(30,msg4.minute);
        ASSERT_EQ(2,msg4.duration_minutes);
        
        ASSERT_EQ(AIS8_001_22_SHAPE_CIRCLE,msg4.sub_areas[0]->getType());
        Ais8_001_22_Circle* sub_area0 = dynamic_cast<Ais8_001_22_Circle*>(msg4.sub_areas[0]);
        ASSERT_EQ(0,sub_area0->radius_m);
        ASSERT_EQ(-70.40821838378906,sub_area0->x);
        ASSERT_EQ(40.02494812011719,sub_area0->y);
        
        ASSERT_EQ(AIS8_001_22_SHAPE_POLYGON,msg4.sub_areas[1]->getType());
        Ais8_001_22_Polygon* sub_area1 = dynamic_cast<Ais8_001_22_Polygon*>(msg4.sub_areas[1]);
        ASSERT_EQ(103000.0,sub_area1->dists_m[0]);
        ASSERT_EQ(114000.0,sub_area1->dists_m[1]);
        ASSERT_EQ(101000.0,sub_area1->dists_m[2]);
        ASSERT_EQ(89.5,sub_area1->angles[0]);
        ASSERT_EQ(0.0,sub_area1->angles[1]);
        ASSERT_EQ(270.0,sub_area1->angles[2]);
        
        ASSERT_EQ(AIS8_001_22_SHAPE_TEXT,msg4.sub_areas[2]->getType());
        Ais8_001_22_Text* sub_area2 = dynamic_cast<Ais8_001_22_Text*>(msg4.sub_areas[2]);
        ASSERT_STREQ("NOAA RW DMA   ",sub_area2->text.c_str());
          
    }
}
