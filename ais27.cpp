// 'K' - 27 - Long-range AIS broadcast message

#include "ais.h"

Ais27::Ais27(const char *nmea_payload, const size_t pad) {
    assert(nmea_payload);
    assert(pad < 6);

    init();

    const size_t num_bits = strlen(nmea_payload) * 6 - pad;

    if (pad != 0 || num_bits != 96) {
      status = AIS_ERR_BAD_BIT_COUNT;
      return;
    }

    bitset<96> bs;
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (27 != message_id) {status = AIS_ERR_WRONG_MSG_TYPE; return;}
    repeat_indicator = ubits(bs, 6, 2);
    mmsi = ubits(bs, 8, 30);

    position_accuracy = bs[38];
    raim = bs[39];
    nav_status = ubits(bs, 40, 4);
    x = sbits(bs, 44, 18) / 600.;
    y = sbits(bs, 62, 17) / 600.;
    sog = ubits(bs, 79, 6);
    cog = ubits(bs, 85, 9);
    // 0 is a current GNSS position.  1 is NOT the current GNSS position
    gnss = !bs[94];
    spare = bs[95];
}
