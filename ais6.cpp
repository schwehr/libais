// Address Binary Message (ABM) 6

#include "ais.h"

#include <iomanip>
#include <cmath>

Ais6::Ais6(const char *nmea_payload, const size_t pad) {
    assert(nmea_payload);
    init();
    const int payload_len = strlen(nmea_payload)*6 - 46; // in bits w/o DAC/FI
    const size_t num_bits = strlen(nmea_payload) * 6 - pad;
    if (num_bits < 88) { status = AIS_ERR_BAD_BIT_COUNT; return; }

    if (payload_len < 0 or payload_len > 952) {
        status = AIS_ERR_BAD_BIT_COUNT;
        return;
    }

    std::bitset<MAX_BITS> bs;
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

  message_id = ubits(bs, 0, 6);
  if (6 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  seq = ubits(bs,38,2);
  mmsi_dest = ubits(bs, 40, 30);
  retransmit = !bs[70];
  spare = bs[71];
  dac = ubits(bs,72,10);
  fi = ubits(bs,82,6);

    // Handle all the byte aligned payload
    for (int i=0; i<payload_len/8; i++) {
        const int start = 88+i*8;
        payload.push_back(ubits(bs,start,8));
    }
    const int remainder = payload_len % 8; // FIX: need to handle spare bits!!
    if (remainder > 0) {
        const int start = (payload_len/8) * 8;
        payload.push_back(ubits(bs, start, remainder));
    }
}


void Ais6::print() {
    std::cout << "AIS_addressed_binary_message: " << message_id
              << "\tmmsi: " << mmsi << "\n"
              << "\tseq: " << seq << "\n"
              << "\tmmsi_dest" << mmsi_dest << "\n"
              << "\tretransmit" << retransmit << "\n"
              << "\tspare" << spare << "\n"
              << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
    std::cout << "\tpayload: ";
    for (std::vector<unsigned char>::iterator i = payload.begin(); i != payload.end(); i++) {
        std::cout << std::hex <<std::setfill('0') << std::setw(2)<< int(*i);
    }
    std::cout << std::dec << std::nouppercase << std::endl;
}


Ais6_1_0::Ais6_1_0(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (88 > num_bits || num_bits > 936) { status = AIS_ERR_BAD_BIT_COUNT; return;  }

  std::bitset<1024> bs;  // TODO: what is the real max size?
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) { return; }

  message_id = ubits(bs, 0, 6);
  if (6 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  seq = ubits(bs,38,2);
  mmsi_dest = ubits(bs, 40, 30);
  retransmit = !bs[70];
  spare = bs[71];
  dac = ubits(bs,72,10);
  fi = ubits(bs,82,6);

  if ( 1 != dac || 0 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  ack_required = bs[88];
  msg_seq = ubits(bs,89,11);

  const size_t text_size = 6 * ((num_bits - 100)/6);
  const size_t spare2_size = num_bits - 100 - text_size;
  text =  ais_str(bs,100,text_size);

  if (!spare2_size) spare2 = 0;
  else spare2 = ubits(bs,100+text_size,spare2_size);
}

void Ais6_1_0::print() {
  std::cout << "ABM_imo_6_1_0: text " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n"
            << "\ttext: " << text << "\n";
}


Ais6_1_1::Ais6_1_1(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if ( num_bits != 112) { status = AIS_ERR_BAD_BIT_COUNT; return;  }

  std::bitset<112> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;

  message_id = ubits(bs, 0, 6);
  if (6 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  seq = ubits(bs,38,2);
  mmsi_dest = ubits(bs, 40, 30);
  retransmit = !bs[70];
  spare = bs[71];
  dac = ubits(bs,72,10);
  fi = ubits(bs,82,6);

  if ( 1 != dac ||  1 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  ack_dac = ubits(bs,88,10);
  msg_seq = ubits(bs,98,11);
  spare2 = ubits(bs,109,3);
}
void Ais6_1_1::print() {
  std::cout << "ABM_imo_6_1_1: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n"
            << "\tack_dac: " << ack_dac << "\tmsg_seq: " << msg_seq << "\n";
}


Ais6_1_2::Ais6_1_2(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 104) { status = AIS_ERR_BAD_BIT_COUNT; return;  }

  std::bitset<104> bs; // TODO: what is the real bit count?
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;

  message_id = ubits(bs, 0, 6);
  if (6 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  seq = ubits(bs,38,2);
  mmsi_dest = ubits(bs, 40, 30);
  retransmit = !bs[70];
  spare = bs[71];
  dac = ubits(bs,72,10);
  fi = ubits(bs,82,6);

  if ( 1 != dac || 2 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  req_dac = ubits(bs,88,10);
  req_fi = ubits(bs,98,6);
}
void Ais6_1_2::print() {
  std::cout << "ABM_imo_6_1_2: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n"
            << "\treq_dac: " << req_dac << "\treq_fi: " << req_fi << "\n";
}


// IFM 3: Capability interrogation - OLD ITU 1371-1
Ais6_1_3::Ais6_1_3(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 104) { status = AIS_ERR_BAD_BIT_COUNT; return;  }

  std::bitset<104> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;

  message_id = ubits(bs, 0, 6);
  if (6 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  seq = ubits(bs,38,2);
  mmsi_dest = ubits(bs, 40, 30);
  retransmit = !bs[70];
  spare = bs[71];
  dac = ubits(bs,72,10);
  fi = ubits(bs,82,6);

  if ( 1 != dac ||  3 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  req_dac = ubits(bs, 88, 10);
  spare2 = ubits(bs, 94, 6);
}
void Ais6_1_3::print() {
  std::cout << "ABM_imo_6_1_3: " << message_id
            << "\t\tdac: " << dac << "\tfi: " << fi << "\treq_dac: "<< req_dac << "\n";
}

// IFM 4: Capability reply - OLD ITU 1371-4
// TODO: WTF?  10 + 128 + 6 == 80  Is this 168 or 232 bits?
Ais6_1_4::Ais6_1_4(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  std::cerr << "TODO: num_bits for 6_1_4: " << num_bits << std::endl;
  // TODO: might also be possible: num_bits != 168
  // TODO: or 226 bits?
  if (num_bits != 232) { status = AIS_ERR_BAD_BIT_COUNT; return;  }

  std::bitset<168> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;

  message_id = ubits(bs, 0, 6);
  if (6 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  seq = ubits(bs,38,2);
  mmsi_dest = ubits(bs, 40, 30);
  retransmit = !bs[70];
  spare = bs[71];
  dac = ubits(bs,72,10);
  fi = ubits(bs,82,6);

  if ( 1 != dac || 4 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  ack_dac = ubits(bs,88,10);
  for (size_t cap_num = 0; cap_num < 128/2; cap_num ++) {
    size_t start = 98 + cap_num * 2;
    capabilities[cap_num] = bs[start];
    cap_reserved[cap_num] = bs[start+1];
  }
  spare2 = ubits(bs,226,6); // OR NOT

  assert(false); // TODO: add in the offset of the dest mmsi
}
void Ais6_1_4::print() {
  std::cout << "ABM_imo_6_1_4: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
  // TODO: implment
}


// IMO Circ 289 - Dangerous cargo
// See also Circ 236
Ais6_1_12::Ais6_1_12(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 360) {
    status = AIS_ERR_BAD_BIT_COUNT; return;  }

  std::bitset<360> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;

  message_id = ubits(bs, 0, 6);
  if (6 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  seq = ubits(bs,38,2);
  mmsi_dest = ubits(bs, 40, 30);
  retransmit = !bs[70];
  spare = bs[71];
  dac = ubits(bs,72,10);
  fi = ubits(bs,82,6);

  if ( 1 != dac || 12 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  assert(false); // TODO: add in the offset of the dest mmsi

  last_port = ais_str(bs, 56, 30);
  utc_month_dep = ubits(bs, 86, 4);
  utc_day_dep = ubits(bs, 90, 5);
  utc_hour_dep = ubits(bs, 95, 5);
  utc_min_dep = ubits(bs, 100, 6);
  next_port = ais_str(bs, 106, 30);
  utc_month_next = ubits(bs, 136, 4); // estimated arrival
  utc_day_next = ubits(bs, 140, 5);
  utc_hour_next = ubits(bs, 145, 5);
  utc_min_next = ubits(bs, 150, 6);
  main_danger = ais_str(bs, 156, 120);
  imo_cat = ais_str(bs, 276, 24);
  un = ubits(bs, 300, 13);
  value = ubits(bs, 313, 10); // UNIT???
  value_unit = ubits(bs, 323, 2);
  spare = ubits(bs, 325, 3);
  // 360
}

void Ais6_1_12::print() {
  std::cout << "ABM_imo_6_1_12_Dangerous cargo: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
  // TODO: implment
}


// 6_1_13 Does not exist

// IMO Circ 289 - Tidal Window
// See also Circ 236
Ais6_1_14::Ais6_1_14(const char *nmea_payload, const size_t pad=0) {
  // TODO: untested - no sample messages of the correct length yet found.
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (376 != num_bits) { status = AIS_ERR_BAD_BIT_COUNT;  return; }

  std::bitset<376> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;

  message_id = ubits(bs, 0, 6);
  if (6 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  seq = ubits(bs,38,2);
  mmsi_dest = ubits(bs, 40, 30);
  retransmit = !bs[70];
  spare = bs[71];
  dac = ubits(bs,72,10);
  fi = ubits(bs,82,6);

  if ( 1 != dac || 14 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  utc_month = ubits(bs, 88, 4);
  utc_day = ubits(bs, 92, 5);

  for (size_t window_num=0; window_num < 3; window_num++) {
    Ais6_1_14_Window w;
    const size_t start = 88 + 9 + window_num * 100;
    // yes, bits are lat, lon
    w.y = sbits(bs, start, 27) / 600000.;
    w.x = sbits(bs, start+27, 28) / 600000.;

    w.utc_hour_from = ubits(bs, start+55, 5);
    w.utc_min_from = ubits(bs, start+60, 6);
    w.utc_hour_to = ubits(bs, start+66, 5);
    w.utc_min_to = ubits(bs, start+71, 6);
    w.cur_dir = ubits(bs, start+77, 9);
    w.cur_speed  = ubits(bs, start+84, 7)/ 10.;

    windows.push_back(w);
  }

}

void Ais6_1_14::print() {
  std::cout << "ABM_imo_6_1_14_Tidal Window: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
  // TODO: implment
}


// IMO Circ 289 - Clearance time to enter port
Ais6_1_18::Ais6_1_18(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (360 != num_bits) { status = AIS_ERR_BAD_BIT_COUNT; return; }

  std::bitset<360> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;

  message_id = ubits(bs, 0, 6);
  if (6 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  seq = ubits(bs,38,2);
  mmsi_dest = ubits(bs, 40, 30);
  retransmit = !bs[70];
  spare = bs[71];
  dac = ubits(bs,72,10);
  fi = ubits(bs,82,6);

  if ( 1 != dac || 18 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  link_id = ubits(bs, 56+32, 10);
  utc_month = ubits(bs, 66+32, 4);
  utc_day = ubits(bs, 70+32, 5);
  utc_hour = ubits(bs, 75+32, 5);
  utc_min = ubits(bs, 80+32, 6);
  port_berth = ais_str(bs, 86+32, 120);
  dest = ais_str(bs, 206+32, 30);
  x = sbits(bs, 236+32, 25) / 60000.; // TODO: check that the diving number is corrent
  y = sbits(bs, 261+32, 24) / 60000.;
  spare2[0] = ubits(bs, 285+32, 32);
  spare2[1] = ubits(bs, 349, 11);
}

void Ais6_1_18::print() {
  std::cout << "ABM_imo_6_1_18_ClearanceToEnterPort: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
  // TODO: implment
}

// IMO Circ 289 - Berthing data
Ais6_1_20::Ais6_1_20(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (360 != num_bits) { status = AIS_ERR_BAD_BIT_COUNT; return; }

  std::bitset<360> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;

  message_id = ubits(bs, 0, 6);
  if (6 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  seq = ubits(bs,38,2);
  mmsi_dest = ubits(bs, 40, 30);
  retransmit = !bs[70];
  spare = bs[71];
  dac = ubits(bs,72,10);
  fi = ubits(bs,82,6);

  if ( 1 != dac || 20 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  link_id = ubits(bs, 88, 10);
  length = ubits(bs, 98, 9);
  depth = ubits(bs, 107, 8);
  position = ubits(bs, 115, 3);
  utc_month = ubits(bs, 118, 4);
  utc_day = ubits(bs, 122, 5);
  utc_hour = ubits(bs, 127, 5);
  utc_min = ubits(bs, 132, 6);
  services_known = bs[138];
  std::cerr << "serv bit decode:";
  for (size_t serv_num=0; serv_num < 26; serv_num++) {
    const int val = ubits(bs, 139 + 2*serv_num, 2);
    std::cerr << val << " ";
    services[serv_num] = int(ubits(bs, 139 + 2*serv_num, 2));
  }
  std::cerr << "\n";
  name = ais_str(bs, 191, 120);;
  x = sbits(bs, 311, 25);
  y = sbits(bs, 336, 24);
}

void Ais6_1_20::print() {
  std::cout << "ABM_imo_6_1_20_Berthing data: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
  // TODO: implment
}


// IMO Circ 289 - Dangerous cargo indication 2
// See also Circ 236
Ais6_1_25::Ais6_1_25(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  // TODO: make sure the bits are a multiple of the size of cargos + header or padded to a slot boundary
  // Allowing a message with no payloads
  // TODO: (num_bits-100) % 17 != 0) is okay
  if (100 > num_bits || num_bits > 576) { status = AIS_ERR_BAD_BIT_COUNT; return; }
  if ( (num_bits - 100) % 17 != 0) { std::cerr << "6_1_25 not 17 aligned;";status = AIS_ERR_BAD_BIT_COUNT; return; }

  std::bitset<576> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;

  message_id = ubits(bs, 0, 6);
  if (6 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  seq = ubits(bs,38,2);
  mmsi_dest = ubits(bs, 40, 30);
  retransmit = !bs[70];
  spare = bs[71];
  dac = ubits(bs,72,10);
  fi = ubits(bs,82,6);


  if ( 1 != dac || 25 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  amount_unit = ubits(bs, 88, 2);
  amount = ubits(bs, 90, 10);
  const size_t total_cargos = int(floor((num_bits - 100) / 17.));
  for (size_t cargo_num=0; cargo_num < total_cargos; cargo_num++) {
    Ais6_1_25_Cargo cargo;
    const size_t start = 100 + 17*cargo_num;
    cargo.code_type = ubits(bs, start, 4);
    cargo.imdg_valid = cargo.spare_valid = cargo.un_valid = cargo.bc_valid = cargo.marpol_oil_valid = cargo.marpol_cat_valid = false;
    // TODO: is this the correct behavior?
    switch(cargo.code_type) {
      // No 0
      case 1:  // IMDG Code in packed form
        cargo.imdg = ubits(bs, start+4, 7); cargo.imdg_valid = true;
        cargo.spare = ubits(bs, start+11, 6); cargo.spare_valid = true;
        break;
      case 2: // IGC Code
        cargo.un = ubits(bs, start+4, 13); cargo.un_valid = true;
        break;
      case 3: // BC Code
        cargo.bc = ubits(bs, start+4, 3); cargo.bc_valid = true;
        cargo.imdg = ubits(bs, start+4+3, 7); cargo.imdg_valid = true;
        cargo.spare = ubits(bs, start+4+3+7, 3); cargo.spare_valid = true;
        break;
      case 4: // MARPOL Annex I
        cargo.marpol_oil= ubits(bs, start+4, 4); cargo.marpol_oil_valid = true;
        cargo.spare = ubits(bs, start+8, 9); cargo.spare_valid = true;
        break;
      case 5: // MARPOL Annex II IBC
        cargo.marpol_cat = ubits(bs, start+4, 3); cargo.marpol_cat_valid = true;
        cargo.spare = ubits(bs, start+7, 10); cargo.spare_valid = true;
        break;
      // 6: Regional use
      // 7: 7-15 reserved for future
      default:
          ; // Just push in an all blank record?
    }
    cargos.push_back(cargo);
  }

}

void Ais6_1_25::print() {
  std::cout << "ABM_imo_6_1_25_DangerousCargoIndication: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
  // TODO: implment
}


// TODO: 6_1_28 - Modify 8_1_28 once that is debugged


// IMO Circ 289 - Tidal window
// See also Circ 236
Ais6_1_32::Ais6_1_32(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  // TODO: might get messages with not all windows.  Might also get 360 bits
  if (350 != num_bits) { status = AIS_ERR_BAD_BIT_COUNT; return; }

  std::bitset<360> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;

  message_id = ubits(bs, 0, 6);
  if (6 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  seq = ubits(bs,38,2);
  mmsi_dest = ubits(bs, 40, 30);
  retransmit = !bs[70];
  spare = bs[71];
  dac = ubits(bs,72,10);
  fi = ubits(bs,82,6);

  if ( 1 != dac || 32 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  utc_month = ubits(bs, 88, 4);
  utc_day = ubits(bs, 92, 5);

  for (size_t window_num=0; window_num < 3; window_num++) {
    Ais6_1_32_Window w;
    const size_t start = 97 + 88*window_num;
    w.x = sbits(bs, start, 25) / 60000.;
    w.y = sbits(bs, start+25, 24) / 60000.;
    w.from_utc_hour = ubits(bs, start+49 ,5);
    w.from_utc_min = ubits(bs, start+54, 6);
    w.to_utc_hour = ubits(bs, start+60, 5);
    w.to_utc_min = ubits(bs, start+65, 6);
    w.cur_dir = ubits(bs, start+71, 9);
    w.cur_speed = ubits(bs, start+80, 8) / 10.;
    windows.push_back(w);
  }
}

void Ais6_1_32::print() {
  std::cout << "ABM_imo_8_1_32_TidalWindow: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
  // TODO: implment
}


// IFM 40: people on board - OLD ITU 1371-4
Ais6_1_40::Ais6_1_40(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 104) { status = AIS_ERR_BAD_BIT_COUNT; return;  }

  std::bitset<104> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;

  message_id = ubits(bs, 0, 6);
  if (6 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  seq = ubits(bs,38,2);
  mmsi_dest = ubits(bs, 40, 30);
  retransmit = !bs[70];
  spare = bs[71];
  dac = ubits(bs,72,10);
  fi = ubits(bs,82,6);

  if ( 1 != dac || 40 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  persons = ubits(bs,88,13);
  spare2 = ubits(bs,101,3);

}
void Ais6_1_40::print() {
  std::cout << "ABM_imo_6_1_40: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n"
            << "\t\tpersons: " << persons << "\n";
}
