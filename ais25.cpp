// Msg 25 - I - Channel Management

#include "ais.h"

// TODO: pad
Ais25::Ais25(const char *nmea_payload) {
    assert(nmea_payload);
    init();

    const int num_char = std::strlen(nmea_payload);
    if (28 > num_char) { status = AIS_ERR_BAD_BIT_COUNT; return; }

    std::bitset<168> bs;
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (25 != message_id) {status = AIS_ERR_WRONG_MSG_TYPE; return;}
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    const bool addressed = bs[38];
    use_app_id = bs[39];
    if (addressed) {
      dest_mmsi_valid = true;
      dest_mmsi = ubits(bs, 40, 30);

      if (use_app_id) {
        dac = ubits(bs,70,10);
        fi = ubits(bs,80,6);
      }
      // TODO: deal with payload

    } else {
      // broadcast
      if (use_app_id) {
        dac = ubits(bs,40,10);
        fi = ubits(bs,50,6);
      }

      // TODO: deal with payload - probably need to pass in the spare bits
    }
}


void Ais25::print() {
    std::cout << "Single slot binary message: " << message_id << "\n"
              << "\tmmsi: " << mmsi << " repeat: " << repeat_indicator << "\n";
    // TODO: finish
}
