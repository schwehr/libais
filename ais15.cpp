// Since 2010-May-14

#include "ais.h"

Ais15::Ais15(const char *nmea_payload) {
    assert(nmea_payload);
    init();

    const int num_char = std::strlen(nmea_payload);
    if (!(num_char >= 14/6 && num_char<=27)) {
        // 88-160 bits
        status = AIS_ERR_BAD_BIT_COUNT;
        return;
    }

    std::bitset<162> bs; // 160 / 6 = 26.66
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (15 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    // FIX: write the rest
}
