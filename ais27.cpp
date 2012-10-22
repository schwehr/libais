// 'K' - 27 - Long-range AIS broadcast message
#include "ais.h"

Ais27::Ais27(const char *nmea_payload, const size_t pad) {
    assert(nmea_payload);
    assert(0==pad);
    init();

    const size_t num_bits = strlen(nmea_payload) * 6 - pad;  assert(96==num_bits);

    if (96 != num_bits) { status = AIS_ERR_BAD_BIT_COUNT; return; }

    std::bitset<96> bs;
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (27 != message_id) {status = AIS_ERR_WRONG_MSG_TYPE; return;}
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    position_accuracy = bs[38];
    raim = bs[39];
    nav_status = ubits(bs, 40, 4);
    x = sbits(bs, 44, 18) / 600.;
    y = sbits(bs, 62, 17) / 600.;
    sog = ubits(bs,79,6);
    cog = ubits(bs,85,9);
    gnss = !bs[94];
    spare = bs[95];
}


void Ais27::print() {
    std::cout << "K - 27 - Long-range AIS broadcast message" << message_id << "\n"
              << "\tmmsi: " << mmsi << " repeat: " << repeat_indicator << "\n";
    // TODO: finish
}
