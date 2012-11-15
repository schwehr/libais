// A - GNSS broacast -
// TODO(schwehr): only partially coded - need to finish
// http://www.itu.int/rec/R-REC-M.823/en
// http://www.iala-aism.org/iala/publications/documentspdf/doc_348_eng.pdf

// In 823, 30 bit words = 24 bits data followed by 6 parity bits.
// Parity bits are left out of the AIS payload?

#include "ais.h"

Ais17::Ais17(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id == 17);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  if (num_bits != 80 && (num_bits < 120 || num_bits > 816)) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<816> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  spare = ubits(bs, 38, 2);

  x = sbits(bs, 40, 18) / 600.;
  y = sbits(bs, 58, 17) / 600.;
  spare2 = ubits(bs, 75, 5);

  // Spec states that there might be no data.
  if (num_bits == 80) {
    gnss_type = station = z_cnt = seq = -1;
    return;
  }

  gnss_type = ubits(bs, 80, 6);
  station = ubits(bs, 86, 10);
  z_cnt = ubits(bs, 96, 13);
  seq = ubits(bs, 109, 3);
#if 0
  // TODO(schwehr): work in progress
  const unsigned int n = ubits(bs, 112, 5);
#endif
  health = ubits(bs, 117, 3);

#if 0
  // TODO(schwehr): work in progress

  // 2 of the n used above
  const size_t remain_bits = num_bits - 120;

  switch (gnss_type) {
  case 1:  // FALLTHROUGH
    // Differential GNSS corrections (full set of satellites)
  case 9:  // Subset differential GNSS corrections
    if (n - 2 != (remain_bits / (24 + 16))) {
      std::cerr << "WARNING: Bad bit count\n";
    }
    std::cout << "17: bits remain: " << num_bits - 120 << " n: " << n << "\n";
    for (size_t i = 0; i < n - 2; i++) {
      const size_t start = 120 + i * (24 + 16);
      std::cout << "\tscale: " << ubits(bs, start + 0, 1) << "\n";
      std::cout << "\tudre: " << ubits(bs, start + 1, 2) << "\n";
      std::cout << "\tsat_id: " << ubits(bs, start + 3, 5) << "\n";
      std::cout << "\tpseudorange_cor: " << ubits(bs, start + 8, 16) << "\n";
      std::cout << "\trate_cor: " << ubits(bs, start + 24, 8) << "\n";
      std::cout << "\tissue: " << ubits(bs, start + 32, 8) << "\n\n";
    }
    break;
  case 3:  // Reference station parameters (GPS)
  case 4:  // Reference station datum
  case 5:  // Constallation health
  case 6:  // Null frame
  case 7:  // Ceacon almanac
  case 16:  // Special message - text
  case 27:  // GPS or GLONASS - Radio Beacon almanac
  case 31:  // Might match GPS 1 - Differential GLONASS
  case 34:  // Might match GPS 6 - Differential GLONASS - same as 31
  case 32:  // Might match GPS 3 - Ref station GLONASS
  case 33:  // Might match GPS 5 - Constellation health GLONASS
  case 35:  // Might match GPS 7 - Beacon almanac (GLONASS)
  case 36:  // Might match GPS 16 - Special GLONASS MESSAGE
    status = AIS_ERR_MSG_SUB_NOT_IMPLEMENTED;
    break;
  default:
    status = AIS_ERR_BAD_SUB_MSG;
  }
#endif

  status = AIS_OK;  // TODO(schwehr): not really okay yet
}


ostream& operator<< (ostream &o, const Ais17 &m) {
    return o << "[" << m.message_id << "]: " << m.mmsi
             << " (" << m.x << ", " << m.y << ") t:"
             << m.gnss_type << ", z:" << m.z_cnt
             << ", d s:" << m.station << ", seq:"
             << m.seq << ", h:" << m.health;
}
