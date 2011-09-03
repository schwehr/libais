// Since Apr 2010

#include "ais.h"

#include <iostream>
#include <bitset>
#include <string>
#include <cassert>
#include <cmath>
#include <cstdlib>

using namespace std;

Ais1_2_3::Ais1_2_3(const char *nmea_payload) {
    assert(nmea_payload);
    assert(nmea_ord_initialized); // Make sure we have the lookup table built
    init();

    if (strlen(nmea_payload) != 168/6) { status = AIS_ERR_BAD_BIT_COUNT; return; }

    std::bitset<168> bs; // 1 slot
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (message_id < 1 or message_id > 3) {
        status = AIS_ERR_WRONG_MSG_TYPE;
        return;
    }

    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);
    nav_status = ubits(bs,38,4);

    /*const int*/ rot_raw = sbits(bs,42,8);
    rot_over_range = abs(rot_raw) > 126 ? true : false ;
    //rot = 4.733 * sqrt(fabs(rot_raw));  // FIX: this was wrong... double check
    rot = pow( (rot_raw/4.733), 2 );
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
    utc_spare = -1;
    slot_offset = -1; slot_offset_valid = false;

    slot_increment = -1; slot_increment_valid = false;
    slots_to_allocate = -1;  slots_to_allocate_valid = false;
    keep_flag = false; keep_flag_valid = false;
   

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
        //std::cout << "expecting 3: " << message_id << std::endl;
        // ITDMA
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
Ais1_2_3::print(bool verbose/*=false*/) {
    std::cout << "Class A Position: " << message_id 
              << std::endl;
    if (!verbose) return;
    cout << "\trow_raw: " << rot_raw << endl;
    cout << "\trot: " << rot << " -> " << (rot_over_range? "greater than": " ") << " " << rot << endl;
    cout << "\tsog: " << sog << endl;
    cout << "\tpos_acc: " << position_accuracy << endl;
    cout << "\tpos_x: " << x << endl;
    cout << "\tpos_y: " << y << endl;
    //cout << "cog_raw: " << ubits(bs,116,12) << endl;
    cout << "\tcog: " << cog << endl;
    cout << "\ttrue_heading:" << true_heading << endl;
    cout << "\ttimestamp: " << timestamp << endl;
    cout << "\tspecial_manoeuvre: " << special_manoeuvre << endl;
    cout << "\tspare: " << spare << endl;
    cout << "\tsync_state: " << sync_state << endl;
    if ( 1 == message_id || 2 == message_id) {
        cout << "\tSOTDMA type " << message_id << endl;
        cout << "\t\tslot_offset: " << slot_offset << endl;
        cout << "\t\tslot_timeout: " << slot_timeout << endl;
        cout << "\t\tslot_number: " << slot_number << endl;
        cout << "\t\treceived_stations: " << received_stations << endl;
    } else {
        assert (3 == message_id);
        cout << "\tITDMA type" << endl;
        cout << "\t\tslot_increment: " << slot_increment << endl;
        cout << "\t\tslots_to_allocate: "  << slots_to_allocate << endl;
        cout << "\t\tkeep_flag: " << (keep_flag?"keep":"do_not_keep") << endl;
    }
}

std::ostream& operator<< (std::ostream& o, Ais1_2_3 const& a)
{
    return o << a.message_id << ": " << a.mmsi ;
}

