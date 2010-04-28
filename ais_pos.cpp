// Since Apr 2010
// g++ ais_pos.cxx -o ais_pos -g -Wall -O3 -Wimplicit -W -Wredundant-decls -pedantic  -funroll-loops -fexpensive-optimizations 

#include <iostream>
#include <bitset>
#include <string>
#include <cassert>
#include <cmath>
using namespace std;

#define UNUSED __attribute((__unused__))

bitset<6> nmea_ord[128];

void build_nmea_lookup() {
    for (int c=0; c < 128; c++) {
         int val = c - 48;
         if (val>=40) val-= 8;
         if (val < 0) continue;
         bitset<6> bits(val);
         bool tmp;
         tmp = bits[5]; bits[5] = bits[0]; bits[0] = tmp;
         tmp = bits[4]; bits[4] = bits[1]; bits[1] = tmp;
         tmp = bits[3]; bits[3] = bits[2]; bits[2] = tmp;
         nmea_ord[c] = bits;

    }
}
 
void aivdm_to_bits(bitset<168> &bits, const char *nmea_payload) {
    for (size_t char_idx=0; char_idx < 28; char_idx++) {
        const bitset<6> bs_for_char = nmea_ord[ int(nmea_payload[char_idx]) ];

        for (size_t offset=0; offset < 6 ; offset++) 
            bits[char_idx*6 + offset] = bs_for_char[offset];
    }
}

class AisPos123 {
public:
    int message_id;
    int repeat_indicator;
    int mmsi;
    int nav_status;
    bool rot_over_range;
    float rot;
    float sog;
    int position_accuracy;
    float x, y; // Long and lat
    float cog;
    int true_heading;
    int timestamp;
    int special_manoeuvre;
    int spare;
    bool raim;

    // COMM state SOTDMA msgs 1 and 2
    int sync_state;
    int slot_timeout;

    // Based on slot_timeout which ones are valid
    int received_stations;
    bool received_stations_valid;
    
    int slot_number;
    bool slot_number_valid;

    bool utc_valid;  // Only means that the values are set.  Can still have 
    int utc_hour;
    int utc_min;
    int utc_spare;

    int slot_offset;
    bool slot_offset_valid;

    // ITDMA - msg type 3
    int slot_increment;
    bool slot_increment_valid;

    int slots_to_allocate;      
    bool slots_to_allocate_valid;

    bool keep_flag;
    bool keep_flag_valid;

    AisPos123(const char *nmea_payload);
};

int ubits(const bitset<168> &bits, const size_t start, const size_t len) 
{
    assert (len <= 32);
    bitset<32> bs_tmp;
    for (size_t i = 0; i < len; i++)
        bs_tmp[i] = bits[start+len-i-1];
    return bs_tmp.to_ulong();
}

typedef union {
    long long_val;
    unsigned long ulong_val;
} long_union;

int sbits(bitset<168> bs, const size_t start, const size_t len) {
    assert (len <= 32);
    bitset<32> bs32;
    if (len < 32 && 1 == bs[start] ) bs32.flip(); // pad 1's to the left if negative

    for (size_t i = 0; i < len; i++)
        bs32[i] = bs[start+len-i-1];

    long_union val;
    val.ulong_val = bs32.to_ulong();
    return val.long_val;
}

AisPos123::AisPos123(const char *nmea_payload) {
    bitset<168> bs;
    aivdm_to_bits(bs, nmea_payload);

    message_id = ubits(bs, 0, 6);
    cout << "message_id: " << message_id << endl;
    assert (message_id >= 1 and message_id <= 3);

    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);
    nav_status = ubits(bs,38,4);
    const int rot_raw = sbits(bs,42,8);
    assert(rot_raw == -128);
    rot_over_range = abs(rot_raw) > 126 ? true : false ;
    rot = 4.733 * sqrt(fabs(rot_raw));
    if (rot_raw < 0) rot = -rot;

    cout << "rot: " << rot_raw << " -> " << (rot_over_range? "greater than": " ") << " " << rot << endl;
    sog = ubits(bs,50,10) / 10.;
    cout << "sog: " << sog << endl;
    position_accuracy = ubits(bs,60,1);
    cout << "pos_acc: " << position_accuracy << endl;
    x = sbits(bs, 61, 28) / 600000.;
    y = sbits(bs, 89, 27) / 600000.;
    cout << "pos_x: " << x << endl;
    cout << "pos_y: " << y << endl;
    cout << "cog_raw: " << ubits(bs,116,12) << endl;
    cog = ubits(bs, 116, 12) / 10.;
    cout << "cog: " << cog << endl;
    true_heading = ubits(bs, 128, 9);
    cout << "true_heading:" << true_heading << endl;
    timestamp = ubits(bs, 137, 6);
    cout << "timestamp: " << timestamp << endl;
    special_manoeuvre = ubits(bs, 143, 2);
    cout << "special_manoeuvre: " << special_manoeuvre << endl;
    spare = ubits(bs, 145, 3);
    cout << "spare: " << spare << endl;
    raim = bool(bs[148]);

    sync_state = ubits(bs, 149, 2);
    cout << "sync_state: " << sync_state << endl;

    // Set all to invalid - this way we don't have to track it in multiple places
    received_stations = -1;  received_stations_valid = false;
    slot_number = -1; slot_number_valid = false;
    utc_hour = utc_min = -1; utc_valid = false;
    slot_offset = -1; slot_offset_valid = false;

    slot_increment = -1; slot_increment_valid = false;
    slots_to_allocate = -1;  slots_to_allocate_valid = false;
    keep_flag = -1; keep_flag_valid = false;
   

    if ( 1 == message_id || 2 == message_id) {
        cout << "SOTDMA type " << message_id << endl;
        slot_timeout = ubits(bs,151,3);
        cout << "slot_timeout: " << slot_timeout << endl;

        switch (slot_timeout) {
        case 0:
            slot_offset = ubits(bs, 154, 14);
            cout << "slot_offset: " << slot_offset << endl;
            break;
        case 1:
            utc_hour = ubits(bs, 154, 5); 
            utc_min = ubits(bs, 159, 7);
            utc_spare = ubits(bs, 166, 2);
            utc_valid = true;
            break;
        case 2: // FALLTHROUGH
        case 4: // FALLTHROUGH
        case 6:
            slot_number = ubits(bs, 154, 14);
            slot_number_valid = true;
            cout << "slot_number: " << slot_number << endl;
            break;
        case 3: // FALLTHROUGH
        case 5: // FALLTHROUGH
        case 7:
            received_stations = ubits(bs, 154, 14);
            received_stations_valid = true;
            cout << "received_stations: " << received_stations << endl;
            break;
        default:
            assert (false);
        }
    } else {
        assert (3 == message_id);
        cout << "ITDMA type" << endl;
        slot_increment = ubits(bs, 151, 13);
        slot_increment_valid = true;

        slots_to_allocate = ubits(bs, 164, 3);
        slots_to_allocate_valid = true;

        keep_flag = bool(bs[167]);
        keep_flag_valid = true;

        cout << "ITDMA: " << slot_increment << " " << slots_to_allocate << " " 
             << (keep_flag?"keep":"do_not_keep") << endl;
    }
}

int main(UNUSED int argc, UNUSED char *argv[]) {
    build_nmea_lookup();
    AisPos123 pos("15Mw9FPP1;Je>`:GDdaC9gv220SA");
    return 0;
}
