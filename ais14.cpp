// Since 2010-May-5
// Safety related broadcast message (SRBM)
#include "ais.h"

Ais14::Ais14
(const char *nmea_payload) {
    assert(nmea_payload);
    init();

    const int num_bits = strlen(nmea_payload)*6;
    if (num_bits < 46 or num_bits > 1008) {
        // FIX: can't check to make sure the string occurs on 6-bit boundaries
        // Need to know the spare bits
        status = AIS_ERR_BAD_BIT_COUNT; 
        std::cout << "num_bits: " << num_bits << "\n";
        return; 
    }

    std::bitset<1008> bs; // 424 + 2 spare bits => 71 characters
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) {
        std::cout << "bit convert failed" << std::endl;
        return;
    }
    
    message_id = ubits(bs, 0, 6);
    if (14 != message_id) { 
        std::cout << "bad id: " << message_id << "\n";
        status = AIS_ERR_WRONG_MSG_TYPE; return; 
    }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    const int num_char = ( num_bits - 40 ) / 6;

    text = ais_str(bs, 40, num_char * 6);
    expected_num_spare_bits = num_bits - 40 - num_char*60; // Can use this to check later
}

void
Ais14::print() {
    std::cout << "Ais14 - Safety broadcast\n"
              << "\tmessage_id: " << message_id << "  " << repeat_indicator << " " << mmsi << "\n"
              << "\ttext: " << text << "\n";
}
