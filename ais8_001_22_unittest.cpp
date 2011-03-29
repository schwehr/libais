// Since 29-Mar-2011 Kurt Schwehr
// Google Test unit testing (started with 1.6.0rc) for the Area Notice
// Based on messages from ais_areanotice_py

#include <gtest/gtest.h>
#include "ais.h"
#include "ais8_001_22.h"

#include <string>

using namespace std;

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
    return str.substr(start+1, end-start);
}

TEST(EmptyTest, Empty) {

}

TEST(PointTest, Point) {
    // AreaNotice: type=0  start=2011-07-06 00:00:00  duration=60 m  link_id=10  sub-areas: 1
    const string msg_str = "!AIVDM,1,1,,A,81mg=5@0EP:0>H0007P>0<D1<qp400000,0*1D";
    const string body(nth_field(msg_str,5,','));
    cout << "body: " << body << endl;
    Ais8_001_22 msg("81mg=5@0EP:0>H0007P>0<D1<qp400000");

} // PointTest
