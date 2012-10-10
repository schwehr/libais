// A - GNSS broacast - FIX: only partially coded

#include "ais.h"


Ais17::Ais17(const char *nmea_payload) {
    assert(nmea_payload);
    init();

    // 80 to 816 bits
    const int num_char = std::strlen(nmea_payload);
    if (num_char < 14 || num_char > 136) {
        status = AIS_ERR_BAD_BIT_COUNT;
        std::cerr << "ais17_num_char: " << num_char << std::endl;
        return;
    }

    std::bitset<168> bs;
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (17 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    spare = ubits(bs,38,2);

    x = sbits(bs, 40, 18) / 600.;
    y = sbits(bs, 58, 17) / 600.;
    spare2 = ubits(bs,75,5);

    if (14 == num_char) {
      // No corrections
      return;
    }
    //std::cerr << "TODO: unpack all the correction data messages" << std::endl;
}
