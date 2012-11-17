// Address Binary Message (ABM) 6

#include <iomanip>
#include <cmath>

#include "ais.h"

Ais6::Ais6(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id == 6);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  const int payload_len = num_bits - 46;  // in bits w/o DAC/FI
  if (num_bits < 88 || payload_len < 0 || payload_len > 952) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<MAX_BITS> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  seq = ubits(bs, 38, 2);
  mmsi_dest = ubits(bs, 40, 30);
  retransmit = !bs[70];
  spare = bs[71];
  dac = ubits(bs, 72, 10);
  fi = ubits(bs, 82, 6);
}

Ais6_1_0::Ais6_1_0(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;
  assert(dac == 1);
  assert(fi == 0);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (88 > num_bits || num_bits > 936) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<1024> bs;  // TODO(schwehr): what is the real max size?
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  ack_required = bs[88];
  msg_seq = ubits(bs, 89, 11);

  const size_t text_size = 6 * ((num_bits - 100) / 6);
  const size_t spare2_size = num_bits - 100 - text_size;
  text =  ais_str(bs, 100, text_size);

  if (!spare2_size)
    spare2 = 0;
  else
    spare2 = ubits(bs, 100 + text_size, spare2_size);

  status = AIS_OK;
}

Ais6_1_1::Ais6_1_1(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 1);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 112) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<112> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  ack_dac = ubits(bs, 88, 10);
  msg_seq = ubits(bs, 98, 11);
  spare2 = ubits(bs, 109, 3);

  status = AIS_OK;
}

Ais6_1_2::Ais6_1_2(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 2);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 104) { status = AIS_ERR_BAD_BIT_COUNT; return;  }

  bitset<104> bs;  // TODO(schwehr): what is the real bit count?
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  req_dac = ubits(bs, 88, 10);
  req_fi = ubits(bs, 98, 6);

  status = AIS_OK;
}

// IFM 3: Capability interrogation - OLD ITU 1371-1
Ais6_1_3::Ais6_1_3(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 3);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 104) { status = AIS_ERR_BAD_BIT_COUNT; return;  }

  bitset<104> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  req_dac = ubits(bs, 88, 10);
  spare2 = ubits(bs, 94, 6);

  status = AIS_OK;
}

// IFM 4: Capability reply - OLD ITU 1371-4
// TODO(schwehr): WTF?  10 + 128 + 6 == 80  Is this 168 or 232 bits?
Ais6_1_4::Ais6_1_4(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;
  assert(dac == 1);
  assert(fi == 4);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  // TODO(schwehr): num_bits for 6_1_4.  226 bits?
  if (num_bits != 232) { status = AIS_ERR_BAD_BIT_COUNT; return;  }

  bitset<168> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  ack_dac = ubits(bs, 88, 10);
  for (size_t cap_num = 0; cap_num < 128/2; cap_num++) {
    size_t start = 98 + cap_num * 2;
    capabilities[cap_num] = bs[start];
    cap_reserved[cap_num] = bs[start + 1];
  }
  // spare2 = ubits(bs, 226, 6);  // OR NOT
  // TODO(schwehr): add in the offset of the dest mmsi

  status = AIS_OK;
}

// IMO Circ 289 - Dangerous cargo
// See also Circ 236
Ais6_1_12::Ais6_1_12(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 12);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 360) {
    status = AIS_ERR_BAD_BIT_COUNT; return;  }

  bitset<360> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  // TODO(schwehr): add in the offset of the dest mmsi

#if 0
  last_port = ais_str(bs, 56, 30);
  utc_month_dep = ubits(bs, 86, 4);
  utc_day_dep = ubits(bs, 90, 5);
  utc_hour_dep = ubits(bs, 95, 5);
  utc_min_dep = ubits(bs, 100, 6);
  next_port = ais_str(bs, 106, 30);
  utc_month_next = ubits(bs, 136, 4);  // estimated arrival
  utc_day_next = ubits(bs, 140, 5);
  utc_hour_next = ubits(bs, 145, 5);
  utc_min_next = ubits(bs, 150, 6);
  main_danger = ais_str(bs, 156, 120);
  imo_cat = ais_str(bs, 276, 24);
  un = ubits(bs, 300, 13);
  value = ubits(bs, 313, 10);  // TODO(schwehr): units
  value_unit = ubits(bs, 323, 2);
  spare = ubits(bs, 325, 3);
  // 360
#endif

  status = AIS_OK;
}

// 6_1_13 Does not exist

// IMO Circ 289 - Tidal Window
// See also Circ 236
Ais6_1_14::Ais6_1_14(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad) {
  // TODO(schwehr): untested - no sample of the correct length yet
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 14);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 376) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<376> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  utc_month = ubits(bs, 88, 4);
  utc_day = ubits(bs, 92, 5);

  for (size_t window_num = 0; window_num < 3; window_num++) {
    Ais6_1_14_Window w;
    const size_t start = 97 + window_num * 93;
    w.y = sbits(bs, start, 27) / 600000.;
    w.x = sbits(bs, start + 27, 28) / 600000.;

    w.utc_hour_from = ubits(bs, start + 55, 5);
    w.utc_min_from = ubits(bs, start + 60, 6);
    w.utc_hour_to = ubits(bs, start + 66, 5);
    w.utc_min_to = ubits(bs, start + 71, 6);
    w.cur_dir = ubits(bs, start + 77, 9);
    w.cur_speed  = ubits(bs, start + 86, 7) / 10.;

    windows.push_back(w);
  }

  status = AIS_OK;
}

// IMO Circ 289 - Clearance time to enter port
Ais6_1_18::Ais6_1_18(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 18);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 360) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<360> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  link_id = ubits(bs, 88, 10);
  utc_month = ubits(bs, 98, 4);
  utc_day = ubits(bs, 102, 5);
  utc_hour = ubits(bs, 107, 5);
  utc_min = ubits(bs, 112, 6);
  port_berth = ais_str(bs, 118, 120);
  dest = ais_str(bs, 238, 30);
  x = sbits(bs, 268, 25) / 60000.;
  y = sbits(bs, 293, 24) / 60000.;
  spare2[0] = ubits(bs, 317, 32);
  spare2[1] = ubits(bs, 349, 11);

  status = AIS_OK;
}

// IMO Circ 289 - Berthing data
Ais6_1_20::Ais6_1_20(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 20);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 360) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<360> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  link_id = ubits(bs, 88, 10);
  length = ubits(bs, 98, 9);
  depth = ubits(bs, 107, 8);
  position = ubits(bs, 115, 3);
  utc_month = ubits(bs, 118, 4);
  utc_day = ubits(bs, 122, 5);
  utc_hour = ubits(bs, 127, 5);
  utc_min = ubits(bs, 132, 6);
  services_known = bs[138];
  for (size_t serv_num = 0; serv_num < 26; serv_num++) {
    // TODO(schwehr): const int val = ubits(bs, 139 + 2*serv_num, 2);
    services[serv_num] = static_cast<int>(ubits(bs, 139 + 2*serv_num, 2));
  }
  name = ais_str(bs, 191, 120);
  x = sbits(bs, 311, 25);
  y = sbits(bs, 336, 24);

  status = AIS_OK;
}

// IMO Circ 289 - Dangerous cargo indication 2
// See also Circ 236
Ais6_1_25::Ais6_1_25(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 25);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  // TODO(schwehr): verify multiple of the size of cargos + header
  //   or padded to a slot boundary
  // Allowing a message with no payloads
  // TODO(schwehr): (num_bits - 100) % 17 != 0) is okay
  if (100 > num_bits || num_bits > 576) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }
  if ((num_bits - 100) % 17 != 0) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<576> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  amount_unit = ubits(bs, 88, 2);
  amount = ubits(bs, 90, 10);
  const size_t total_cargos = static_cast<int>(floor((num_bits - 100) / 17.));
  for (size_t cargo_num = 0; cargo_num < total_cargos; cargo_num++) {
    Ais6_1_25_Cargo cargo;
    const size_t start = 100 + 17*cargo_num;
    cargo.code_type = ubits(bs, start, 4);
    cargo.imdg_valid = false;
    cargo.spare_valid = false;
    cargo.un_valid = false;
    cargo.bc_valid = false;
    cargo.marpol_oil_valid = false;
    cargo.marpol_cat_valid = false;

    // TODO(schwehr): is this the correct behavior?
    switch (cargo.code_type) {
      // No 0
      case 1:  // IMDG Code in packed form
        cargo.imdg = ubits(bs, start + 4, 7);
        cargo.imdg_valid = true;
        cargo.spare = ubits(bs, start + 11, 6);
        cargo.spare_valid = true;
        break;
      case 2:  // IGC Code
        cargo.un = ubits(bs, start + 4, 13);
        cargo.un_valid = true;
        break;
      case 3:  // BC Code
        cargo.bc = ubits(bs, start + 4, 3);
        cargo.bc_valid = true;
        cargo.imdg = ubits(bs, start + 7, 7);
        cargo.imdg_valid = true;
        cargo.spare = ubits(bs, start + 14, 3);
        cargo.spare_valid = true;
        break;
      case 4:  // MARPOL Annex I
        cargo.marpol_oil= ubits(bs, start + 4, 4);
        cargo.marpol_oil_valid = true;
        cargo.spare = ubits(bs, start + 8, 9);
        cargo.spare_valid = true;
        break;
      case 5:  // MARPOL Annex II IBC
        cargo.marpol_cat = ubits(bs, start + 4, 3);
        cargo.marpol_cat_valid = true;
        cargo.spare = ubits(bs, start + 7, 10);
        cargo.spare_valid = true;
        break;
      // 6: Regional use
      // 7: 7-15 reserved for future
      default:
        {}  // Just push in an all blank record?
    }
    cargos.push_back(cargo);
  }

  status = AIS_OK;
}

// TODO(schwehr): 6_1_28 - Modify 8_1_28 once that is debugged

// IMO Circ 289 - Tidal window
// See also Circ 236
Ais6_1_32::Ais6_1_32(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 32);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  // TODO(schwehr): might get messages with not all windows
  if (num_bits != 350) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<360> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  utc_month = ubits(bs, 88, 4);
  utc_day = ubits(bs, 92, 5);

  for (size_t window_num = 0; window_num < 3; window_num++) {
    Ais6_1_32_Window w;
    const size_t start = 97 + 88*window_num;
    w.x = sbits(bs, start, 25) / 60000.;
    w.y = sbits(bs, start + 25, 24) / 60000.;
    w.from_utc_hour = ubits(bs, start + 49, 5);
    w.from_utc_min = ubits(bs, start + 54, 6);
    w.to_utc_hour = ubits(bs, start + 60, 5);
    w.to_utc_min = ubits(bs, start + 65, 6);
    w.cur_dir = ubits(bs, start + 71, 9);
    w.cur_speed = ubits(bs, start + 80, 8) / 10.;
    windows.push_back(w);
  }

  status = AIS_OK;
}

// IFM 40: people on board - OLD ITU 1371-4
Ais6_1_40::Ais6_1_40(const char *nmea_payload, const size_t pad)
    : Ais6(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 40);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 104) { status = AIS_ERR_BAD_BIT_COUNT; return;  }

  bitset<104> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  persons = ubits(bs, 88, 13);
  spare2 = ubits(bs, 101, 3);

  status = AIS_OK;
}
