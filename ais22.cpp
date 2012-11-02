// Msg 22 - F - Channel Management

#include "ais.h"

Ais22::Ais22(const char *nmea_payload, const size_t pad) {
    assert(nmea_payload);
    assert(pad < 6);
    init();

    if (pad != 0 || std::strlen(nmea_payload) != 28) {
      status = AIS_ERR_BAD_BIT_COUNT;
      return;
    }

    bitset<168> bs;
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (message_id != 22) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs, 6, 2);
    mmsi = ubits(bs, 8, 30);
    spare = ubits(bs, 38, 2);

    chan_a = ubits(bs, 40, 12);
    chan_b = ubits(bs, 52, 12);
    txrx_mode = ubits(bs, 64, 4);
    power_low = bs[68];

    // WARNING: OUT OF ORDER DECODE
    bool addressed = bs[139];

    if (!addressed) {
      // geographic position
      pos_valid = true;
      dest_valid = false;
      x1 = sbits(bs, 69, 28) / 600000.;
      y1 = sbits(bs, 87, 27) / 600000.;
      x2 = sbits(bs, 104, 28) / 600000.;
      y2 = sbits(bs, 122, 27) / 600000.;
    } else {
      pos_valid = false;
      dest_valid = true;
      dest_mmsi_1 = ubits(bs, 69, 30);
      // 5 spare bits
      dest_mmsi_2 = ubits(bs, 104, 30);
      // 5 spare bits
    }

    // OUT OF ORDER: addressed is earlier before
    chan_a_bandwidth = bs[140];
    chan_b_bandwidth = bs[141];
    zone_size = ubits(bs, 142, 3);

    spare2 = ubits(bs, 145, 23);
}
