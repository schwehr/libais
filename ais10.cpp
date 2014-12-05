// : UTC and date query

#include "ais.h"

Ais10::Ais10(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id == 10);

  if (pad != 0 || strlen(nmea_payload) != 12) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<72> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  spare = ubits(bs, 38, 2);
  dest_mmsi = ubits(bs, 40, 30);
  spare2 = ubits(bs, 70, 2);

  status = AIS_OK;
}

ostream& operator<< (ostream &o, const Ais10 &msg) {
  return o << msg.message_id << ": " << msg.mmsi
           << " dest=" << msg.dest_mmsi
           << " " << msg.spare << " " << msg.spare2;
}
