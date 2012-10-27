// : UTC and date query

#include "ais.h"

Ais10::Ais10(const char *nmea_payload, const size_t pad) {
    assert(nmea_payload);
    init();

    if (pad != 0 || strlen(nmea_payload) != 72/6) { status = AIS_ERR_BAD_BIT_COUNT; return; }

    std::bitset<72> bs;
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (10 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs, 6, 2);
    mmsi = ubits(bs, 8, 30);

    spare = ubits(bs, 38, 2);
    dest_mmsi = ubits(bs, 40, 30);
    spare2 = ubits(bs, 70, 2);
}
