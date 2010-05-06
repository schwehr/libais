// AIS message 4 or 11

#include "ais.h"

#include <iostream>
#include <bitset>
#include <string>
#include <cassert>
//#include <cmath>

Ais4_11::Ais4_11(const char *nmea_payload) {
    assert(nmea_payload);
    init();

    std::bitset<168> bs; 

    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (message_id != 4 and message_id != 11) {
        status = AIS_ERR_WRONG_MSG_TYPE;
        return;
    }

    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    year = ubits(bs,38,14);
    month = ubits(bs,52,4);
    day = ubits(bs,56,5);
    hour = ubits(bs,61,5);
    minute = ubits(bs,66,6);
    second = ubits(bs,72,6);

    position_accuracy = ubits(bs,78,1);
    x = sbits(bs, 79, 28) / 600000.;
    y = sbits(bs, 107, 27) / 600000.;

    fix_type = ubits(bs,134,4);
    spare = ubits(bs,138,10);


    //
    // SOTDMA commstate
    //

    // Set all to invalid - this way we don't have to track it in multiple places
    received_stations = -1;  received_stations_valid = false;
    slot_number = -1; slot_number_valid = false;
    utc_hour = utc_min = -1; utc_valid = false;
    slot_offset = -1; slot_offset_valid = false;

    slot_timeout = ubits(bs,151,3);

    //std::cout << "slot_timeout:" << slot_timeout << std::endl;
    switch (slot_timeout) {
    case 0:
        slot_offset = ubits(bs, 154, 14);
        slot_offset_valid=true;
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
}

void Ais4_11::print() {
    std::cout << ( ( 4==message_id )?"bs_report: ":"utc response: ") << message_id << "\n"
              << "\tmmsi: " << mmsi << " repeat: " << repeat_indicator << "\n"
              << "\tdate/time: " << year << "-" << month << "-" << day
              << "T" << hour << ":" << minute << ":" << second << "Z\n"
              << "\tposition_accuracy: " << position_accuracy << "\n"
              << "\tlocation: " << x << " " << y << "\n"
              << "\tspare: " << spare << "\n"
              << "\traim: " << (raim?"true":"false") << "\n"
              << std::endl;
        ;
        
}

std::ostream& operator<< (std::ostream& o, Ais4_11 const& msg)
{
    return o << msg.message_id << ": " << msg.mmsi 
        ;
}
