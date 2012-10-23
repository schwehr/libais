// Since 2010-May-14
#include "ais.h"

// TODO: pad
Ais12::Ais12(const char *nmea_payload) {
    assert(nmea_payload);
    init();

    const int num_char = strlen(nmea_payload);
    // WARNING: the spec says max of 1008 bits, but 168 + 4*256 => 1192 bits or 199 characters
    if ( num_char < 12 || num_char > 199 ) { status = AIS_ERR_BAD_BIT_COUNT; return; }

    std::bitset<1192> bs; // Spec says 1008
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (12 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    seq_num = ubits(bs,38,2);
    dest_mmsi = ubits(bs,40,30);
    retransmitted = bs[70];
    spare = bs[71];
    int num_txt_bits = 6 * ((num_char * 6 - 72) / 6);
    text = ais_str(bs,72,num_txt_bits);
}
