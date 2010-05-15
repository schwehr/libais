// Since 2010-May-14

#include "ais.h"

Ais10::Ais10(const char *nmea_payload) {
    assert(nmea_payload);
    init();

    if (strlen(nmea_payload) != 72/6) {
        std::cout << "WHAT? " << strlen(nmea_payload) << 72/6 << std::endl;
        status = AIS_ERR_BAD_BIT_COUNT;
        return;
    }

    std::bitset<72> bs; // 1 slot
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (10 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    spare = ubits(bs,38,2);
    dest_mmsi = ubits(bs,40,30);
    spare2 = ubits(bs,70,2);
}

void Ais10::print() {
    std::cout << "UTC and date inquiry: " << message_id << "\n"
              << "\tmmsi: " << mmsi << " repeat: " << repeat_indicator << "\n"
              << "\tdest_mmsi: " << dest_mmsi << " " << "spare2: " << spare2 << "\n";
}
