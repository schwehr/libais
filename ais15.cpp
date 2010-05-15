// Since 2010-May-14

#include "ais.h"

Ais9::Ais9(const char *nmea_payload) {
    assert(nmea_payload);
    init();

    const int num_char = str_len(nmea_payload);
    if !(num_char == 162/6 || num_char== ) {
      status = AIS_ERR_BAD_BIT_COUNT;
      return;
    }

    std::bitset<162> bs; // 160 / 6 = 26.66
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (9 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);
