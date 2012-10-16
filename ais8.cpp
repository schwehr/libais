// Since 2010-05-19
// Binary Broadcast Message (BBM) - 8

#include "ais.h"

#include <cmath>
#include <iomanip>

Ais8::Ais8(const char *nmea_payload) {
    assert(nmea_payload);
    init();
    assert(nmea_ord_initialized); // Make sure we have the lookup table built
    
    const int payload_len = strlen(nmea_payload)*6 - 46; // in bits w/o DAC/FI
    //std::cout << "payload_len: " << strlen(nmea_payload) << " " << strlen(nmea_payload)*6 << " " << payload_len << " " << payload_len / 8 << "\n";
    if (payload_len < 0 or payload_len > 952) {
        status = AIS_ERR_BAD_BIT_COUNT;
        return;
    }

    std::bitset<MAX_BITS> bs;  // FIX: shouldn't this be a max of 1192?
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;  // checks status

    if (!decode_header8(bs)) return; // side effect - sets status

    // Handle all the byte aligned payload
    for (int i=0; i<payload_len/8; i++) {
        const int start = 56+i*8;
        //std::cout << "payload: " << i << " " << start <<"\n";
        payload.push_back(ubits(bs,start,8));
    }
    const int remainder = payload_len % 8; // FIX: need to handle spare bits!!
    //std::cout << "remainder: " << remainder << "\n";
    if (remainder > 0) {
        const int start = (payload_len/8) * 8;
        //std::cout << "start: " << start <<"\n";
        payload.push_back(ubits(bs, start, remainder));
    }
}

bool Ais8::decode_header8(const std::bitset<MAX_BITS> &bs) {
    message_id = ubits(bs, 0, 6);
    if (8 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return false; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);
    spare = ubits(bs,38,2);
    dac = ubits(bs,40,10);
    fi = ubits(bs,50,6);
    return true;
}

void Ais8::print() {
    std::cout << "AIS_broadcast_binary_message: " << message_id
              << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
    std::cout << "\tpayload: "; // << std::hex << std::uppercase; // << std::setfill('0') << std::setw(2) << "\n";
    for (std::vector<unsigned char>::iterator i = payload.begin(); i != payload.end(); i++) {
        std::cout << std::hex <<std::setfill('0') << std::setw(2)<< int(*i);
    }
    std::cout << std::dec << std::nouppercase << std::endl;
    //std::cout << "test: " << 1 << " " << 255 << " " << std::hex << 255 << std::endl;
}


//////////////////////////////////////////////////////////////////////
Ais8_1_11::Ais8_1_11(const char *nmea_payload, const size_t pad) {
    assert(nmea_payload);  assert(0 <= pad && pad <= 7);
    init();
    //CHECKPOINT;

    if (strlen(nmea_payload) != 59) {  status = AIS_ERR_BAD_BIT_COUNT; return;  }

    std::bitset<354> bs;  // 352 + 2 spares to be 6 bit aligned
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;  // checks status
    // CHECKPOINT;

    //decode_header8(bs);
    message_id = ubits(bs, 0, 6);
    if (8 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    spare = ubits(bs,38,2);
    dac = ubits(bs,40,10);
    fi = ubits(bs,50,6);

    //CHECKPOINT;
    // FIX: if dac is not 001, it could still possibly be correct
    if ( 1 != dac || 11 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

    y = sbits(bs,56,24) / 60000.; // YES, lat is first
    x = sbits(bs,80,25) / 60000.;
    day = ubits(bs,105,5);
    hour = ubits(bs,110,5);
    minute = ubits(bs,115,6);
    wind_ave = ubits(bs,121,7);
    wind_gust = ubits(bs,128,7);
    wind_dir = ubits(bs,135,9);
    wind_gust_dir = ubits(bs,144,9);
    air_temp = ubits(bs,153,11) / 10. - 60;
    rel_humid = ubits(bs,164,7);
    dew_point = ubits(bs,171,10) / 10. - 20; // FIX: please be right
    air_pres = ubits(bs,181,9) + 800;
    air_pres_trend = ubits(bs,190,2);
    horz_vis = ubits(bs,192,8) / 10.;
    water_level = ubits(bs,200,9) / 10. - 10; // FIX: please be right for -10.0 to 30.0
    water_level_trend = ubits(bs,209,2);
    surf_cur_speed = ubits(bs,211,8) / 10.;
    surf_cur_dir = ubits(bs,219,9);
    cur_speed_2 = ubits(bs,228,8) / 10.;
    cur_dir_2 = ubits(bs,236,9);
    cur_depth_2 = ubits(bs,245,5);
    cur_speed_3 = ubits(bs,250,8) / 10.;
    cur_dir_3 = ubits(bs,258,9);
    cur_depth_3 = ubits(bs,267,5);

    wave_height = ubits(bs,272,8) / 10.;
    wave_period = ubits(bs,280,6);
    wave_dir = ubits(bs,286,9);
    swell_height = ubits(bs,295,8) / 10.;
    swell_period = ubits(bs,303,6);
    swell_dir = ubits(bs,309,9);

    sea_state = ubits(bs,318,4);
    water_temp = ubits(bs,322,10) / 10. - 10; // FIX: please be right for -10.0 to +50.0
    precip_type = ubits(bs,332,3);
    salinity = ubits(bs,335,9);
    ice = ubits(bs,344,2);
    extended_water_level = ubits(bs,346,6); // FIX: how to treat this???
}

void Ais8_1_11::print() {
    std::cout << "BBM_imo_1_11_met_hydro: " << message_id
              << "\t\tdac: " << dac << "\tfi:" << fi << "\n";

    std::cout << "\tspare:" << spare << "\n"
              << "\tlocation:" <<  x << " " <<  y << "\t(lon, lat WGS84)" << "\n"
              << "\ttime:" <<  day << "T" << hour << ":" << minute << "Z\n"
              << "\twind:" <<  wind_ave << " " << wind_dir << "\n"
              << "\tgust:" <<  wind_gust << " " << wind_gust_dir << "\n"
              << "\tair_temp:" <<  air_temp << "\thumidity:" <<  rel_humid << "\t:" <<  horz_vis << "\n"
              << "\tdew_point:" <<  dew_point << "\tair_pres:" <<  air_pres << " trend: " << air_pres_trend << "\n"
              << "\twater_level:" <<  water_level << " trend: " << water_level_trend << "\n"
              << "\tcurrent:" <<  surf_cur_speed << " kts dir: " << surf_cur_dir << "\n"
              << "\tcurr2:" <<  cur_speed_2 << " kts dir: " << cur_dir_2 << "\t " <<  cur_depth_2 << " m deep\n"
              << "\tcurr3:" <<  cur_speed_3 << " kts dir: " << cur_dir_3 << "\t " <<  cur_depth_3 << " m deep\n"
              << "\twaves:" <<  wave_height << " " << wave_period << " " <<  wave_dir << "\n"
              << "\tswell:" <<  swell_height << " " <<  swell_period << " " <<  swell_dir << "\n"
              << "\tsea_state:" <<  sea_state << "\twater_temp:" <<  water_temp << "\n"
              << "\tprecip_type:" <<  precip_type << "\tsalinity:" <<  salinity << "\n"
              << "\tice:" <<  ice << "\n"
              << "\tspare_or_extended_wl:" <<  extended_water_level << std::endl;
}


// No 8_1_12


// IMO Circ 289 - Fairway Closed
// See also Circ 236
Ais8_1_13::Ais8_1_13(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  //const int num_char = strlen(nmea_payload);

  if (472 != num_bits) { status = AIS_ERR_BAD_BIT_COUNT; return; }

  std::bitset<472> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;  // checks status

  //decode_header8(bs);
  message_id = ubits(bs, 0, 6);
  if (8 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  spare = ubits(bs,38,2);  // TODO: has meaning?
  dac = ubits(bs,40,10);
  fi = ubits(bs,50,6);

  // TODO: what counties use their own dac/fi waters?  Please do NOT do that.
  if ( 1 != dac || 13 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  reason = ais_str(bs, 56, 120);
  location_from = ais_str(bs, 176, 120);
  location_to = ais_str(bs, 296, 120);
  radius = ubits(bs, 416, 10);
  units = ubits(bs, 426, 2);
  // utc?  warning: day/month out of order
  day_from = ubits(bs, 428, 5);
  month_from = ubits(bs, 433, 4);
  hour_from = ubits(bs, 437, 5);
  minute_from = ubits(bs, 442, 6);
  day_to = ubits(bs, 448, 5);
  month_to = ubits(bs, 453, 4);
  hour_to = ubits(bs, 457, 5);
  minute_to = ubits(bs, 462, 6);
  spare2 = ubits(bs, 468, 4);
}

void Ais8_1_13::print() {
  std::cout << "BBM_imo_8_1_13_FairwayClosed: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
  // TODO: implment
}


// No 8_1_16


// IMO Circ 289 - Extended Shipdata - Air gap
// See also Circ 236
Ais8_1_15::Ais8_1_15(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  //const int num_char = strlen(nmea_payload);

  if (72 != num_bits) { status = AIS_ERR_BAD_BIT_COUNT; return; }

  std::bitset<72> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;  // checks status

  //decode_header8(bs);
  message_id = ubits(bs, 0, 6);
  if (8 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  spare = ubits(bs,38,2);  // TODO: has meaning?
  dac = ubits(bs,40,10);
  fi = ubits(bs,50,6);

  // TODO: what counties use their own dac/fi waters?  Please do NOT do that.
  if ( 1 != dac || 15 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  air_draught = ubits(bs, 56, 11) / 10.;
  spare2 = ubits(bs, 66, 5);
}

void Ais8_1_15::print() {
  std::cout << "BBM_imo_1_8_15_Extended Shipdata - Air gap: " << message_id
            << "\t\tdac: " << dac << "	fi:" << fi << "\n"
            << "\t\tair_draught (m): " << air_draught << std::endl;
}



// IMO Circ 289 - Number of persons on board
// See also Circ 236
// TODO: there might also be an addressed version?
Ais8_1_16::Ais8_1_16(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  //const int num_char = strlen(nmea_payload);

  if (72 == num_bits) { status = AIS_ERR_BAD_BIT_COUNT; return; }

  std::bitset<72> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;  // checks status

  //header_decode8(bs);
  message_id = ubits(bs, 0, 6);
  if (8 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  spare = ubits(bs,38,2);  // TODO: has meaning?
  dac = ubits(bs,40,10);
  fi = ubits(bs,50,6);


  // TODO: what counties use their own dac/fi waters?  Please do NOT do that.
  if ( 1 != dac || 16 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  persons = ubits(bs, 56, 13);
  spare2 = ubits(bs, 69, 3);
}

void Ais8_1_16::print() {
  std::cout << "BBM_imo_1_8_16_Number of persons on board: " << message_id
            << "\t\tdac: " << dac << "	fi:" << fi << "\n"
            << "\t\tpersons: " << persons << std::endl;
}



// IMO Circ 289 - VTS Generated/Synthetic Targets
// See also Circ 236
Ais8_1_17::Ais8_1_17(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  //const int num_char = strlen(nmea_payload);

  if (56 != num_bits && 176 != num_bits && 295 != num_bits && 416 != num_bits && 536 != num_bits) {
    status = AIS_ERR_BAD_BIT_COUNT; return;
  }

  std::bitset<536> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;  // checks status

  // decode_header8(bs);
  message_id = ubits(bs, 0, 6);
  if (8 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  spare = ubits(bs,38,2);  // TODO: has meaning?
  dac = ubits(bs,40,10);
  fi = ubits(bs,50,6);


  // TODO: what counties use their own dac/fi waters?  Please do NOT do that.
  if ( 1 != dac || 17 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  const size_t num_targets = (num_bits - 56) / 120;
  for (size_t target_num=0; target_num < num_targets; target_num++) {
    Ais8_1_17_Target target;
    const size_t start = 56 + (120 * target_num);
    target.type = ubits(bs, start , 2);
    target.id = ais_str(bs,  start+2 , 42);
    target.spare = ubits(bs, start+44 , 4);
    target.y = sbits(bs, start+48 , 24) / 60000.;  // booo - lat, lon
    target.x = sbits(bs, start+72 , 25) / 60000.;
    target.cog = ubits(bs, start+97 , 9);
    target.timestamp = ubits(bs, start+106 , 6);
    target.sog = ubits(bs, start+112 , 8);
  }
}

void Ais8_1_17::print() {
  std::cout << "BBM_imo_1_8_17_VTS Generated/Synthetic Targets: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
  // TODO: implment
}


// No msg 8_1_18


// IMO Circ 289 - Marine traffic signal
Ais8_1_19::Ais8_1_19(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  //const int num_char = strlen(nmea_payload);

  // I bet some people transmit without the idiodic spare padding
  if (258 != num_bits && 360 != num_bits) { status = AIS_ERR_BAD_BIT_COUNT; return; }

  std::bitset<360> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;  // checks status

  // decode_header8(bs);
  message_id = ubits(bs, 0, 6);
  if (8 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  spare = ubits(bs,38,2);  // TODO: has meaning?
  dac = ubits(bs,40,10);
  fi = ubits(bs,50,6);


  // TODO: what counties use their own dac/fi waters?  Please do NOT do that.
  if ( 1 != dac || 19 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  link_id = ubits(bs, 56, 10);
  name = ais_str(bs, 66, 120);
  x = sbits(bs, 186, 25) / 60000.;
  y = sbits(bs, 211, 24) / 60000.;
  status = ubits(bs, 235, 2);
  signal = ubits(bs, 237, 5);
  utc_hour_next = ubits(bs, 242, 5);
  utc_min_next = ubits(bs, 247, 6);
  next_signal = ubits(bs, 253, 5);
  if (360 != num_bits) return;
  spare2[0] = ubits(bs, 258, 32);
  spare2[1] = ubits(bs, 290, 32);
  spare2[2] = ubits(bs, 322, 32);
  spare2[3] = ubits(bs, 354, 6);
}

void Ais8_1_19::print() {
  std::cout << "BBM_imo_8_1_19_Marine traffic signal: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
  // TODO: implment
}


// No 8_1_20



// IMO Circ 289 - Weather observation report from ship
// See also Circ 236
Ais8_1_21::Ais8_1_21(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  //const int num_char = strlen(nmea_payload);

  if (360 != num_bits) { status = AIS_ERR_BAD_BIT_COUNT; return; }

  std::bitset<360> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;  // checks status

  // decode_header8(bs);
  message_id = ubits(bs, 0, 6);
  if (8 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  spare = ubits(bs,38,2);  // TODO: has meaning?
  dac = ubits(bs,40,10);
  fi = ubits(bs,50,6);


  // TODO: what counties use their own dac/fi waters?  Please do NOT do that.
  if ( 1 != dac || 21 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  type_wx_report = bs[56];
  if (!type_wx_report) {
    // WX obs from ship
    location = ais_str(bs, 57, 120);
    x = sbits(bs, 177, 25) / 60000.;
    y = sbits(bs, 202, 24) / 60000.;
    utc_day = ubits(bs, 226, 5);
    utc_hour = ubits(bs, 231, 5);
    utc_min = ubits(bs, 236, 6);
    wx[0] = ubits(bs, 242, 4);  // TODO: set wx[1] and wx[2]?
    horz_viz = ubits(bs, 246, 8) / 10.; // nautical miles
    humidity = ubits(bs, 254, 7);  // %
    wind_speed = ubits(bs, 261, 7);  // ave knots
    wind_dir = ubits(bs, 268, 9);
    pressure = ubits(bs, 277, 9); // hPa
    pressure_tendency = ubits(bs, 286, 4);
    // TODO: is air_temp correct?
    air_temp = sbits(bs, 290, 11) / 10.; // C
    water_temp = ubits(bs, 301, 10) / 10. - 10; // C
    wave_period = ubits(bs, 311, 6); // s
    wave_height = ubits(bs, 317, 8) / 10.;
    wave_dir = ubits(bs, 325, 9);
    swell_height = ubits(bs, 334, 8) / 10.; // m
    swell_dir = ubits(bs, 342, 9);
    swell_period = ubits(bs, 351, 6); // s
    spare2 = ubits(bs, 357, 3);
  } else {
    // type == 1
    // PAIN IN THE ASS WMO OBS from ship
    // TODO: double check the insanity
    x = (ubits(bs, 57, 16) / 100.) - 180;
    y = (ubits(bs, 73, 15) / 100.) - 180;

    utc_month = ubits(bs, 88, 4);
    utc_day = ubits(bs, 92, 6);
    utc_hour = ubits(bs, 98, 5);
    utc_min = ubits(bs, 102, 3) * 10;
    cog = ubits(bs, 106, 7) * 5;
    sog = ubits(bs, 113, 5) * 0.5;
    heading = ubits(bs, 118, 7) *5;  // Assume this is true degrees????
    pressure = ubits(bs, 125, 11) /10. +900;
    rel_pressure = ubits(bs, 136, 10) / 10. -50;
    pressure_tendency = ubits(bs, 146, 4);
    wind_dir = ubits(bs, 150, 7) * 5;
    // 6+2+30+2+16 +1 +16+15 +4+6+5+3+7+5+7+11+10+4+7 == 157

    wind_speed_ms = ubits(bs, 157, 8) * 0.5; // m/s
    wind_dir_rel = ubits(bs, 165, 7) * 5;
    wind_speed_rel= ubits(bs, 172, 8) * 0.5; // m/s
    wind_gust_speed = ubits(bs, 180, 8) * 0.5; // m/s
    wind_gust_dir = ubits(bs, 188, 7) * 5;
    // 0C = 273.15 Kelvin
    // TODO: change this to celcius
    air_temp_raw = ubits(bs, 195, 10);
    humidity =ubits(bs, 205, 7);
    water_temp_raw = ubits(bs, 212, 9);  // TODO: change this to C.  Jerks.
    horz_viz = pow(ubits(bs, 221, 6), 2) * 13.073; // WTF?  the result is meters.
    wx[0] = ubits(bs, 227, 9); // current
    wx[1] = ubits(bs, 236, 5); // past 1
    wx[2] = ubits(bs, 241, 5); // past 2
    // 6+2+30+2+16 +1 +16+15 +4+6+5+3+7+5+7+11+10+4+7 +8+7+8+8+7+10+7+9+6+9+5+5 == 246

    cloud_total = ubits(bs, 246, 4) * 10;
    cloud_low = ubits(bs, 250, 4);
    cloud_low_type = ubits(bs, 254, 6);
    cloud_middle_type = ubits(bs, 260, 6);
    cloud_high_type = ubits(bs, 266, 6);
    alt_lowest_cloud_base = pow(ubits(bs, 272, 7), 2) * 0.16; // Jerks
    wave_period = ubits(bs, 279, 5); // s
    wave_height = ubits(bs, 284, 6) * 0.5; // m
    swell_dir = ubits(bs, 290, 6) * 10;
    swell_period = ubits(bs, 296, 5); // s
    swell_height = ubits(bs, 301, 6) * 0.5; // m
    swell_dir_2 = ubits(bs, 307, 6) * 10;
    // 6+2+30+2+16 +1 +16+15 +4+6+5+3+7+5+7+11+10+4+7 +8+7+8+8+7+10+7+9+6+9+5+5 +4+4+6+6+6+7+5+6+6+5+6+6 == 313

    swell_period_2 = ubits(bs, 313, 5); // s
    swell_height_2 = ubits(bs, 318, 6) * 0.5; // m
    ice_thickness = ubits(bs, 324, 7) / 100.; // network is cm, storing m
    ice_accretion = ubits(bs, 331, 3);
    ice_accretion_cause = ubits(bs, 334, 3);
    sea_ice_concentration = ubits(bs, 337, 5);
    amt_type_ice = ubits(bs, 342, 4);
    ice_situation = ubits(bs, 346, 5);
    ice_devel = ubits(bs, 351, 5);
    bearing_ice_edge = ubits(bs, 356, 4) * 45;
    // 6+2+30+2+16 +1 +16+15 +4+6+5+3+7+5+7+11+10+4+7 +8+7+8+8+7+10+7+9+6+9+5+5 +4+4+6+6+6+7+5+6+6+5+6+6 +5+6+7+3+3+5+4+5+5+4 == 360

  }

}

void Ais8_1_21::print() {
  std::cout << "BBM_imo_8_1_21_Weather observation report from ship: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
  // TODO: implment
}



// IMO Circ 289 - Extended ship static and voyage-related
// See also Circ 236
Ais8_1_24::Ais8_1_24(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  //const int num_char = strlen(nmea_payload);

  if (360 != num_bits) { status = AIS_ERR_BAD_BIT_COUNT; return; }

  std::bitset<360> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;  // checks status

  // decode_header8(bs);
  message_id = ubits(bs, 0, 6);
  if (8 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  spare = ubits(bs,38,2);  // TODO: has meaning?
  dac = ubits(bs,40,10);
  fi = ubits(bs,50,6);


  // TODO: what counties use their own dac/fi waters?  Please do NOT do that.
  if ( 1 != dac || 24 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  link_id = ubits(bs, 56, 10);
  air_draught = ubits(bs, 66, 13) / 10.; // m
  last_port = ais_str(bs, 79, 30);
  next_ports[0] = ais_str(bs, 109, 30);
  next_ports[1] = ais_str(bs, 139, 30);

  // TODO enum list of param types
  // 0 NA, 1 operational, 2 SNAFU, 3 no data
  for (size_t equip_num=0; equip_num < 26; equip_num++) {
    solas_status[equip_num] = ubits(bs, 169 + 2 * equip_num, 2);
  }
  ice_class = ubits(bs, 221, 4);
  shaft_power = ubits(bs, 225, 18); // horses
  vhf = ubits(bs, 243, 12);
  lloyds_ship_type = ais_str(bs, 255, 42);
  gross_tonnage = ubits(bs, 297, 18);
  laden_ballast = ubits(bs, 315, 2);
  heavy_oil = ubits(bs, 317, 2);
  light_oil = ubits(bs, 319, 2);
  diesel = ubits(bs, 321, 2);
  bunker_oil = ubits(bs, 323, 14);  // tonnes
  persons = ubits(bs, 337, 13);
  spare2 = ubits(bs, 350, 10);
}

void Ais8_1_24::print() {
  std::cout << "BBM_imo_8_1_24_Extended ship static and voyage-related: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
  // TODO: implment
}


// There is no 8_1_25


// IMO Circ 289 - Environmental
// See also Circ 236
Ais8_1_26::Ais8_1_26(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  //const int num_char = strlen(nmea_payload);

  // TODO: make sure the message is a multiple of waypoints or on slot boundaries

  if (56 < num_bits || num_bits > 1008) { status = AIS_ERR_BAD_BIT_COUNT; return; }

  std::bitset<1008> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;

  // decode_header8(bs);
  message_id = ubits(bs, 0, 6);
  if (8 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  spare = ubits(bs,38,2);  // TODO: has meaning?
  dac = ubits(bs,40,10);
  fi = ubits(bs,50,6);


  // TODO: what counties use their own dac/fi waters?  Please do NOT do that.
  if ( 1 != dac || 26 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  // TODO: implement
  assert(false);
}

void Ais8_1_26::print() {
  std::cout << "BBM_imo_1_8_26_Environmental: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
  // TODO: implment
}



// IMO Circ 289 - Route information
// See also Circ 236
Ais8_1_27::Ais8_1_27(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  //const int num_char = strlen(nmea_payload);

  if (172 < num_bits || num_bits > 997) { status = AIS_ERR_BAD_BIT_COUNT; return; }

  std::bitset<997> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;  // checks status

  // decode_header8(bs);
  message_id = ubits(bs, 0, 6);
  if (8 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  spare = ubits(bs,38,2);  // TODO: has meaning?
  dac = ubits(bs,40,10);
  fi = ubits(bs,50,6);

  // TODO: what counties use their own dac/fi waters?  Please do NOT do that.
  if ( 1 != dac || 27 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  link_id = ubits(bs, 56, 10);
  sender_type = ubits(bs, 66, 3);
  route_type = ubits(bs, 69, 5);
  utc_month = ubits(bs, 74, 4);
  utc_day = ubits(bs, 78, 5);
  utc_hour = ubits(bs, 83, 5);
  utc_min = ubits(bs, 88, 6);
  duration = ubits(bs, 94, 18);
  const size_t num_waypoints = ubits(bs, 112, 5);
  for (size_t waypoint_num=0; waypoint_num < num_waypoints; waypoint_num++) {
    AisPoint pt;
    const size_t start = 117 + 55*waypoint_num;
    pt.x =  sbits(bs, start, 0) / 600000.;
    pt.y =  sbits(bs, start, 28) / 600000.;
    waypoints.push_back(pt);
  }
}

void Ais8_1_27::print() {
  std::cout << "BBM_imo_1_8_27_Route information: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
  // TODO: implment
}

// No 8_1_28


// IMO Circ 289 - Text description
// See also Circ 236
Ais8_1_29::Ais8_1_29(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  //const int num_char = strlen(nmea_payload);

  if (72 < num_bits || num_bits > 1032) { status = AIS_ERR_BAD_BIT_COUNT; return; }

  std::bitset<1032> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;  // checks status

  // decode_header8(bs);
  message_id = ubits(bs, 0, 6);
  if (8 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  spare = ubits(bs,38,2);  // TODO: has meaning?
  dac = ubits(bs,40,10);
  fi = ubits(bs,50,6);


  // TODO: what counties use their own dac/fi waters?  Please do NOT do that.
  if ( 1 != dac || 29 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  link_id = ubits(bs, 56, 10);
  size_t text_bits = num_bits - 66;
  if (text_bits % 6) std::cerr << "WARNING: unhandled spare bits" << std::endl;
  text = ais_str(bs, 66, text_bits);
}

void Ais8_1_29::print() {
  std::cout << "BBM_imo_1_8_29_Text description: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n"
            << "\t\tlink_id: " << link_id << "\ttext: " << text << std::endl;
}



// IMO Circ 289 - Meteorological and Hydrographic data
// See also Circ 236
Ais8_1_31::Ais8_1_31(const char *nmea_payload, const size_t pad=0) {
  assert(nmea_payload);
  init();

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  //const int num_char = strlen(nmea_payload);

  if (360 != num_bits) { status = AIS_ERR_BAD_BIT_COUNT; return; }

  std::bitset<360> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;  // checks status

  // decode_header8(bs);
  message_id = ubits(bs, 0, 6);
  if (8 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
  repeat_indicator = ubits(bs,6,2);
  mmsi = ubits(bs,8,30);
  spare = ubits(bs,38,2);  // TODO: has meaning?
  dac = ubits(bs,40,10);
  fi = ubits(bs,50,6);


  // TODO: what counties use their own dac/fi waters?  Please do NOT do that.
  if ( 1 != dac || 31 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  x = sbits(bs,56,25) / 60000.;
  y = sbits(bs,81,24) / 60000.;
  position_accuracy = bs[105];
  utc_day = ubits(bs, 106, 5);
  utc_hour = ubits(bs, 111, 5);
  utc_min = ubits(bs, 116, 6);

  wind_ave = ubits(bs, 122, 7); // kts
  wind_gust = ubits(bs, 129, 7); // kts
  wind_dir = ubits(bs, 136, 9);
  wind_gust_dir = ubits(bs, 145,9);
  air_temp = sbits(bs, 154,11) / 10. ; // C
  rel_humid = ubits(bs, 165,7);
  dew_point = sbits(bs, 172,10)/ 10.;  // TODO: How is this mapped?
  air_pres = ubits(bs, 182,9);
  air_pres_trend = ubits(bs, 191,2);

  horz_vis = ubits(bs, 193, 8) / 10.; // NM
  water_level = ubits(bs, 201, 12) / 100. - 10; // m
  water_level_trend = ubits(bs, 213, 2);

  surf_cur_speed = ubits(bs, 215, 8) / 10.;
  surf_cur_dir = ubits(bs, 223, 9);
  cur_speed_2 = ubits(bs, 232, 8) / 10.; // kts
  cur_dir_2 = ubits(bs, 240, 9);
  cur_depth_2 = ubits(bs, 249, 5); // m
  cur_speed_3 = ubits(bs, 254, 8) / 10.; // kts
  cur_dir_3 = ubits(bs, 262, 9);
  cur_depth_3 = ubits(bs, 271, 5); // m

  wave_height = ubits(bs, 276, 8); // m

  wave_period = ubits(bs, 284, 6);
  wave_dir = ubits(bs, 290, 9);
  swell_height = ubits(bs, 299, 8) / 10.;
  swell_period = ubits(bs, 307, 6);
  swell_dir = ubits(bs, 313, 9);
  sea_state = ubits(bs, 322, 4); // beaufort scale - Table 1.2
  water_temp = sbits(bs, 326, 10) / 10.;
  precip_type = ubits(bs, 336, 3);
  salinity = ubits(bs, 339, 9) / 10.;
  ice = ubits(bs, 348, 2); // yes/no/undef/unknown
  spare = ubits(bs, 350, 10);
}

void Ais8_1_31::print() {
  std::cout << "BBM_imo_8_1_31_MetHydrographic: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
  // TODO: implment
}
