// Msg 20 D - data link management
#include "ais.h"

Ais20::Ais20(const char *nmea_payload) {
    assert(nmea_payload);

    init();

    const int num_char = std::strlen(nmea_payload);
    if (num_char != 12 and num_char != 18 and num_char != 23 and num_char != 27) {
        status = AIS_ERR_BAD_BIT_COUNT;
        //std::cerr << "ERROR Ais20 num_char " << num_char << std::endl;
        return;
    }

    std::bitset<160> bs; // 1 + a partial slot
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (20 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    spare = ubits(bs,38,2);

    // TODO: make this an array of blocks with 4 element

    // TODO: Are the ever no blocks set????

    offset_1 = ubits(bs,40,12);
    num_slots_1 = ubits(bs,52,4);
    timeout_1 = ubits(bs,56,3);
    incr_1 = ubits(bs,59,11);

    if (12 == num_char) {
        group_valid_2 = group_valid_3 = group_valid_4 = false;
        spare2 = ubits(bs, 70, 2);  // Makes the result 8 bit / 1 byte aligned
        return;
    }

    group_valid_2 = true;
    offset_2 = ubits(bs,70,12);
    num_slots_2 = ubits(bs,82,4);
    timeout_2 = ubits(bs,86,3);
    incr_2 = ubits(bs,89,11);
    if (18 == num_char) {
      group_valid_3 = group_valid_4 = false;
      spare2 = ubits(bs, 100, 4);  // Makes the result 8 bit / 1 byte aligned
      return;
    }

    group_valid_3 = true;
    offset_3 = ubits(bs,100,12);
    num_slots_3 = ubits(bs,112,4);
    timeout_3 = ubits(bs,116,3);
    incr_3 = ubits(bs,119,11);
    if (23 == num_char) {
        spare2 = ubits(bs, 130, 6);  // Makes the result 8 bit / 1 byte aligned
    }

    group_valid_4 = true;
    offset_4 = ubits(bs,130,12);
    num_slots_4 = ubits(bs,142,4);
    timeout_4 = ubits(bs,146,3);
    incr_4 = ubits(bs,149,11);

    spare2 = 0;
}

void
Ais20::print() {
    std::cout << "D' - Data link management: " << message_id
              << "\tmmsi: " << mmsi << " repeat: " << repeat_indicator << "\n"
              << "\tspare: " << spare << "\n";
    // TODO: flush out the rest
}
