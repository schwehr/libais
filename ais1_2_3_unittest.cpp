#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include "ais.h"

using namespace std;

TEST(TestAis1_2_3,AisMsg) {
    AisMsg a;
    a.init();
    ASSERT_EQ(AIS_OK, a.get_error());
}

/* Python:
import binary
bv = binary.ais6tobitvec('15Mq4J0P01EREODRv4@74gv00HRq')
print bv
 */
TEST(TestAis1_2_3, BitDecoding) {
    build_nmea_lookup();
    const string m_str("15Mq4J0P01EREODRv4@74gv00HRq");
    const string bits_expected("000001000101011101111001000100011010000000100000000000000001010101100010010101011111010100100010111110000100010000000111000100101111111110000000000000011000100010111001");
    bitset<168> bs;
    AIS_STATUS status = aivdm_to_bits(bs, m_str.c_str());
    //cout << "decode_status: " << status << endl;
    ASSERT_EQ(AIS_OK,status);
    for (size_t i=0; i < 168; i++) {
        char c = bs[i]? '1':'0';
        ASSERT_EQ(bits_expected[i], c);
    }
}

/*
~/projects/src/noaadata/ais/ais_msg_1_handcoded.py -d '!AIVDM,1,1,,B,15Mq4J0P01EREODRv4@74gv00HRq,0*72,b003669970,1272412824'
position:
	MessageID:          1
	RepeatIndicator:    0
	UserID:             366888040
	NavigationStatus:   0
	ROT:                -128
	SOG:                0.1
	PositionAccuracy:   0
	longitude:          -146.2903833333333333333333333
	latitude:           61.11413333333333333333333333
	COG:                181
	TrueHeading:        511
	TimeStamp:          0
	RegionalReserved:   0
	Spare:              0
	RAIM:               False
	sync_state:                   0
	slot_timeout:                 6
	received_stations:            n/a
	slot_number:                  2233
	commstate_utc_hour:           n/a
	commstate_utc_min:            n/a
	commstate_utc_spare:          n/a
	slot_offset:                  n/a
*/

TEST(TestAis1_2_3, AisMsg1) {
    const string m_str("AIVDM,1,1,,B,15Mq4J0P01EREODRv4@74gv00HRq,0*72,b003669970,1272412824");
    const string body(nth_field(m_str,5,','));
    ASSERT_STREQ("15Mq4J0P01EREODRv4@74gv00HRq",body.c_str());
    Ais1_2_3 m(body.c_str());
    //cout << "AisMsg had_error should be: " << AIS_OK << " == " << m.get_error() << endl;
    ASSERT_EQ(AIS_OK, m.get_error());
    //m.print(true);

    ASSERT_EQ(1,m.message_id);
    ASSERT_EQ(0,m.repeat_indicator);
    ASSERT_EQ(366888040,m.mmsi);
    ASSERT_EQ(0,m.nav_status);

    ASSERT_EQ(true,m.rot_over_range);
    ASSERT_EQ(-128,m.rot_raw);
    //ASSERT_FLOAT_EQ(-731.386, m.rot);
    ASSERT_NEAR(-731.386, m.rot, 0.001);

    ASSERT_FLOAT_EQ(0.1,m.sog);

    ASSERT_EQ(false,m.position_accuracy);
    ASSERT_FLOAT_EQ(-146.2903833,m.x);
    ASSERT_FLOAT_EQ(  61.1141333,m.y);

    ASSERT_FLOAT_EQ(181.0,m.cog);
    ASSERT_EQ(511, m.true_heading);
    ASSERT_EQ(0, m.timestamp);
    ASSERT_EQ(0, m.special_manoeuvre); // Pre ITU 1371-3, this and the spare were one
    ASSERT_EQ(0, m.spare);
    ASSERT_EQ(false, m.raim);

    // COMM STATE
    ASSERT_EQ(0,m.sync_state);
    ASSERT_EQ(6,m.slot_timeout);

    ASSERT_EQ(false,m.received_stations_valid);
    ASSERT_EQ(-1,m.received_stations);

    ASSERT_EQ(true,m.slot_number_valid);
    ASSERT_EQ(2233,m.slot_number);

    ASSERT_EQ(false,m.utc_valid);
    ASSERT_EQ(-1,m.utc_hour);
    ASSERT_EQ(-1,m.utc_min);
    ASSERT_EQ(-1,m.utc_spare);

    ASSERT_EQ(false,m.slot_offset_valid);
    ASSERT_EQ(-1,m.slot_offset);

    ASSERT_EQ(false,m.slot_increment_valid);
    ASSERT_EQ(-1,m.slot_increment);

    ASSERT_EQ(false,m.slots_to_allocate_valid);
    ASSERT_EQ(-1,m.slots_to_allocate);

    ASSERT_EQ(false,m.keep_flag_valid);
    ASSERT_EQ(false,m.keep_flag);
}
