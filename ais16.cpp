// @ - Assigned mode command

#include "ais.h"


Ais16::Ais16(const char *nmea_payload) {
    assert(nmea_payload);
    init();

    // 96 or 144 bits
    // 168 bits violates the spec but is common
    const int num_char = std::strlen(nmea_payload);
    if (num_char != 16 && num_char != 24 && num_char != 28) {
        status = AIS_ERR_BAD_BIT_COUNT;
        //std::cerr << "num_char: " << num_char << std::endl;
        return;
    }

    std::bitset<168> bs;
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (16 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    spare = ubits(bs,38,2);

    dest_mmsi_a = ubits(bs,40,30);
    offset_a = ubits(bs,70,12);
    inc_a = ubits(bs,82,10);
    if (num_char == 16) {
      dest_mmsi_b = offset_b = inc_b = -1;  // TODO: Or would 0 be better?
      spare2 = ubits(bs,92,4);
      return;
    }

    dest_mmsi_b = ubits(bs,92,30);
    offset_b = ubits(bs,122,12);
    inc_b = ubits(bs,134,10);
    spare2 = -1;  // TODO: Or would 0 be better?
}
