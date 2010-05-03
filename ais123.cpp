// Since Apr 2010
// g++ ais_pos.cxx -o ais_pos -g -Wall -O3 -Wimplicit -W -Wredundant-decls -pedantic  -funroll-loops -fexpensive-optimizations 

#include "ais.h"

#include <iostream>
#include <bitset>
#include <string>
#include <cassert>
#include <cmath>
//using namespace std;


Ais1_2_3::Ais1_2_3(const char *nmea_payload) {
    //build_nmea_lookup();

    std::bitset<168> bs; // 1 slot
    aivdm_to_bits(bs, nmea_payload);
    std::cout << bs;

    message_id = ubits(bs, 0, 6);
    assert (message_id >= 1 and message_id <= 3);

    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);
    nav_status = ubits(bs,38,4);

    const int rot_raw = sbits(bs,42,8);
    assert(rot_raw == -128);
    rot_over_range = abs(rot_raw) > 126 ? true : false ;
    rot = 4.733 * sqrt(fabs(rot_raw));
    if (rot_raw < 0) rot = -rot;

    sog = ubits(bs,50,10) / 10.;
    position_accuracy = ubits(bs,60,1);
    x = sbits(bs, 61, 28) / 600000.;
    y = sbits(bs, 89, 27) / 600000.;
    cog = ubits(bs, 116, 12) / 10.;
    true_heading = ubits(bs, 128, 9);
    timestamp = ubits(bs, 137, 6);
    special_manoeuvre = ubits(bs, 143, 2);
    spare = ubits(bs, 145, 3);
    raim = bool(bs[148]);

    sync_state = ubits(bs, 149, 2);

    // Set all to invalid - this way we don't have to track it in multiple places
    received_stations = -1;  received_stations_valid = false;
    slot_number = -1; slot_number_valid = false;
    utc_hour = utc_min = -1; utc_valid = false;
    slot_offset = -1; slot_offset_valid = false;

    slot_increment = -1; slot_increment_valid = false;
    slots_to_allocate = -1;  slots_to_allocate_valid = false;
    keep_flag = -1; keep_flag_valid = false;
   

    if ( 1 == message_id || 2 == message_id) {
        slot_timeout = ubits(bs,151,3);

        switch (slot_timeout) {
        case 0:
            slot_offset = ubits(bs, 154, 14);
            slot_offset_valid = true;
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
            break;
        case 3: // FALLTHROUGH
        case 5: // FALLTHROUGH
        case 7:
            received_stations = ubits(bs, 154, 14);
            received_stations_valid = true;
            break;
        default:
            assert (false);
        }
    } else {
        assert (3 == message_id);
        slot_increment = ubits(bs, 151, 13);
        slot_increment_valid = true;

        slots_to_allocate = ubits(bs, 164, 3);
        slots_to_allocate_valid = true;

        keep_flag = bool(bs[167]);
        keep_flag_valid = true;

    }
}

void 
Ais1_2_3::print() {
    std::cout << "Class A Position: " << message_id 
              << std::endl;
    //cout << "rot: " << rot_raw << " -> " << (rot_over_range? "greater than": " ") << " " << rot << endl;
    //cout << "sog: " << sog << endl;
    //cout << "pos_acc: " << position_accuracy << endl;
    //cout << "pos_x: " << x << endl;
    //cout << "pos_y: " << y << endl;
    //cout << "cog_raw: " << ubits(bs,116,12) << endl;
    //cout << "cog: " << cog << endl;
    //cout << "true_heading:" << true_heading << endl;
    //cout << "timestamp: " << timestamp << endl;
    //cout << "special_manoeuvre: " << special_manoeuvre << endl;
    //cout << "spare: " << spare << endl;
    //cout << "sync_state: " << sync_state << endl;
    if ( 1 == message_id || 2 == message_id) {
        //cout << "SOTDMA type " << message_id << endl;
        //cout << "slot_offset: " << slot_offset << endl;
        //cout << "slot_timeout: " << slot_timeout << endl;
        //cout << "slot_number: " << slot_number << endl;
        //cout << "received_stations: " << received_stations << endl;
    } else {
        assert (3 == message_id);
        //cout << "ITDMA type" << endl;
        //        cout << "ITDMA: " << slot_increment << " " << slots_to_allocate << " " 
        //<< (keep_flag?"keep":"do_not_keep") << endl;
    }
}

std::ostream& operator<< (std::ostream& o, Ais1_2_3 const& a)
{
    return o << a.message_id << ": " << a.mmsi ;
}

