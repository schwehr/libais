// Msg 23 - G - Channel Management
#include "ais.h"

Ais23::Ais23(const char *nmea_payload) {
    assert(nmea_payload);
    init();

    const int num_char = std::strlen(nmea_payload);
    if (27 != num_char) { status = AIS_ERR_BAD_BIT_COUNT; return; }

    std::bitset<168> bs;
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (23 != message_id) {status = AIS_ERR_WRONG_MSG_TYPE; return;}
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);
    spare = ubits(bs,38,2);

    x1 = sbits(bs, 40, 18) / 600.;
    y1 = sbits(bs, 58, 17) / 600.;
    x2 = sbits(bs, 75, 18) / 600.;
    y2 = sbits(bs, 93, 17) / 600.;

    station_type = ubits(bs, 110, 4);
    type_and_cargo = ubits(bs, 114, 8);
    spare2 = ubits(bs, 122, 22);

    txrx_mode = ubits(bs, 144, 2);
    interval_raw = ubits(bs, 146, 4);
    quiet = ubits(bs, 148, 4);
    spare3 = ubits(bs, 152, 6);
}


void Ais23::print() {
    //CHECKPOINT;
    std::cout << "Group assignment command: " << message_id << "\n"
              << "\tmmsi: " << mmsi << " repeat: " << repeat_indicator << "\n";
    // TODO: finish
}
