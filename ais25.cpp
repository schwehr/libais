// Msg 25 - I - Single slot binary message

#include "ais.h"

Ais25::Ais25(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id == 25);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  if (num_bits < 40 || num_bits > 168) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<168> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  const bool addressed = bs[38];
  use_app_id = bs[39];
  if (addressed) {
    dest_mmsi_valid = true;
    dest_mmsi = ubits(bs, 40, 30);
    if (use_app_id) {
      dac = ubits(bs, 70, 10);
      fi = ubits(bs, 80, 6);
    }
    // TODO(schwehr): Handle the payloads.
  } else {
    // broadcast
    if (use_app_id) {
      dac = ubits(bs, 40, 10);
      fi = ubits(bs, 50, 6);
    }
    // TODO(schwehr): Handle the payloads.
  }
  status = AIS_OK;
}
