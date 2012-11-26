// Since 2010-05-19
// Binary Broadcast Message (BBM) - 8

#include <cmath>
#include <iomanip>

#include "ais.h"

Ais8::Ais8(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad) {
  // Ais8 is used in some apps as a standalone, so be extra
  // careful to make sure we have the lookup table built
  assert(nmea_ord_initialized);
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id == 8);

  // in bits w/o DAC/FI
  const int payload_len = strlen(nmea_payload) * 6 - 46 - pad;
  if (payload_len < 0 || payload_len > 952) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<MAX_BITS> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  spare = ubits(bs, 38, 2);
  dac = ubits(bs, 40, 10);
  fi = ubits(bs, 50, 6);
}

Ais8_1_0::Ais8_1_0(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 0);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (56 > num_bits || num_bits > 1024) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<MAX_BITS> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  ack_required = bs[56];
  msg_seq = ubits(bs, 57, 11);

  const size_t text_size = 6 * ((num_bits - 68)/6);
  // wrong?  needs to land on 8-bit boundary
  const size_t spare2_size = num_bits - 68 - text_size;
  text = ais_str(bs, 68, text_size);

  // TODO(schwehr): Is this correct?
  if (!spare2_size)
    spare2 = 0;
  else
    spare2 = ubits(bs, 68, spare2_size);
  status = AIS_OK;
}

Ais8_1_11::Ais8_1_11(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 11);

  if (strlen(nmea_payload) != 59) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<354> bs;  // 352 + 2 spares to be 6 bit aligned
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  y = sbits(bs, 56, 24) / 60000.;
  x = sbits(bs, 80, 25) / 60000.;
  day = ubits(bs, 105, 5);
  hour = ubits(bs, 110, 5);
  minute = ubits(bs, 115, 6);
  wind_ave = ubits(bs, 121, 7);
  wind_gust = ubits(bs, 128, 7);
  wind_dir = ubits(bs, 135, 9);
  wind_gust_dir = ubits(bs, 144, 9);
  air_temp = ubits(bs, 153, 11) / 10. - 60;
  rel_humid = ubits(bs, 164, 7);
  dew_point = ubits(bs, 171, 10) / 10. - 20;  // TODO(schwehr): verify
  air_pres = ubits(bs, 181, 9) + 800;
  air_pres_trend = ubits(bs, 190, 2);
  horz_vis = ubits(bs, 192, 8) / 10.;
  // TODO(schwehr): verify for -10.0 to 30.0
  water_level = ubits(bs, 200, 9) / 10. - 10;
  water_level_trend = ubits(bs, 209, 2);
  surf_cur_speed = ubits(bs, 211, 8) / 10.;
  surf_cur_dir = ubits(bs, 219, 9);
  cur_speed_2 = ubits(bs, 228, 8) / 10.;
  cur_dir_2 = ubits(bs, 236, 9);
  cur_depth_2 = ubits(bs, 245, 5);
  cur_speed_3 = ubits(bs, 250, 8) / 10.;
  cur_dir_3 = ubits(bs, 258, 9);
  cur_depth_3 = ubits(bs, 267, 5);

  wave_height = ubits(bs, 272, 8) / 10.;
  wave_period = ubits(bs, 280, 6);
  wave_dir = ubits(bs, 286, 9);
  swell_height = ubits(bs, 295, 8) / 10.;
  swell_period = ubits(bs, 303, 6);
  swell_dir = ubits(bs, 309, 9);

  sea_state = ubits(bs, 318, 4);
  // TODO(schwehr): verify for -10.0 to +50.0
  water_temp = ubits(bs, 322, 10) / 10. - 10;
  precip_type = ubits(bs, 332, 3);
  salinity = ubits(bs, 335, 9);
  ice = ubits(bs, 344, 2);
  // There is no way to know which meaning to attach to the following 6 bits
  // TODO(schwehr): how to treat this spare vrs water level?
  spare2 = ubits(bs, 346, 6);
  extended_water_level = ubits(bs, 346, 6);

  status = AIS_OK;
}

// No 8_1_12

// IMO Circ 289 - Fairway Closed
// See also Circ 236
Ais8_1_13::Ais8_1_13(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;
  assert(dac == 1);
  assert(fi == 13);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 472) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<472> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  reason = ais_str(bs, 56, 120);
  location_from = ais_str(bs, 176, 120);
  location_to = ais_str(bs, 296, 120);
  radius = ubits(bs, 416, 10);
  units = ubits(bs, 426, 2);
  day_from = ubits(bs, 428, 5);
  month_from = ubits(bs, 433, 4);
  hour_from = ubits(bs, 437, 5);
  minute_from = ubits(bs, 442, 6);
  day_to = ubits(bs, 448, 5);
  month_to = ubits(bs, 453, 4);
  hour_to = ubits(bs, 457, 5);
  minute_to = ubits(bs, 462, 6);
  spare2 = ubits(bs, 468, 4);
  status = AIS_OK;
}

// No 8_1_16

// IMO Circ 289 - Extended Shipdata - Air gap
// See also Circ 236
Ais8_1_15::Ais8_1_15(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 15);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 72) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<72> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  air_draught = ubits(bs, 56, 11) / 10.;
  spare2 = ubits(bs, 67, 5);
  status = AIS_OK;
}

// IMO Circ 289 - Number of persons on board
// See also Circ 236
// TODO(schwehr): there might also be an addressed version?
Ais8_1_16::Ais8_1_16(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 16);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 72) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<72> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  persons = ubits(bs, 56, 13);
  spare2 = ubits(bs, 69, 3);
  status = AIS_OK;
}

// IMO Circ 289 - VTS Generated/Synthetic Targets
// See also Circ 236
Ais8_1_17::Ais8_1_17(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 17);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  const size_t num_targets = (num_bits - 56) / 120;
  const size_t extra_bits = (num_bits - 56) % 120;

  if (extra_bits || num_targets > 4) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<536> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  for (size_t target_num = 0; target_num < num_targets; target_num++) {
    Ais8_1_17_Target target;
    const size_t start = 56 + (120 * target_num);
    target.type = ubits(bs, start, 2);
    target.id = ais_str(bs,  start + 2, 42);
    target.spare = ubits(bs, start + 44, 4);
    target.y = sbits(bs, start + 48, 24) / 60000.;  // booo - lat, lon
    target.x = sbits(bs, start + 72, 25) / 60000.;
    target.cog = ubits(bs, start + 97, 9);
    target.timestamp = ubits(bs, start + 106, 6);
    target.sog = ubits(bs, start + 112, 8);
  }
  status = AIS_OK;
}

// No msg 8_1_18

// IMO Circ 289 - Marine traffic signal
Ais8_1_19::Ais8_1_19(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 19);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  // Some people transmit without the idiodic spare padding
  if (num_bits != 258 && num_bits != 360) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<360> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  link_id = ubits(bs, 56, 10);
  name = ais_str(bs, 66, 120);
  x = sbits(bs, 186, 25) / 60000.;
  y = sbits(bs, 211, 24) / 60000.;
  status = ubits(bs, 235, 2);
  signal = ubits(bs, 237, 5);
  utc_hour_next = ubits(bs, 242, 5);
  utc_min_next = ubits(bs, 247, 6);
  next_signal = ubits(bs, 253, 5);
  if (num_bits != 360)
    return;
  spare2[0] = ubits(bs, 258, 32);
  spare2[1] = ubits(bs, 290, 32);
  spare2[2] = ubits(bs, 322, 32);
  spare2[3] = ubits(bs, 354, 6);
  status = AIS_OK;
}

// No 8_1_20

// IMO Circ 289 - Weather observation report from ship
// See also Circ 236
Ais8_1_21::Ais8_1_21(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 21);

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

  type_wx_report = bs[56];
  if (!type_wx_report) {
    // WX obs from ship
    location = ais_str(bs, 57, 120);
    x = sbits(bs, 177, 25) / 60000.;
    y = sbits(bs, 202, 24) / 60000.;
    utc_day = ubits(bs, 226, 5);
    utc_hour = ubits(bs, 231, 5);
    utc_min = ubits(bs, 236, 6);
    wx[0] = ubits(bs, 242, 4);  // TODO(schwehr): set wx[1] and wx[2]?
    horz_viz = ubits(bs, 246, 8) / 10.;  // nautical miles
    humidity = ubits(bs, 254, 7);  // %
    wind_speed = ubits(bs, 261, 7);  // ave knots
    wind_dir = ubits(bs, 268, 9);
    pressure = ubits(bs, 277, 9);  // hPa
    pressure_tendency = ubits(bs, 286, 4);
    // TODO(schwehr): is air_temp correct?
    air_temp = sbits(bs, 290, 11) / 10.;  // C
    water_temp = ubits(bs, 301, 10) / 10. - 10;  // C
    wave_period = ubits(bs, 311, 6);  // s
    wave_height = ubits(bs, 317, 8) / 10.;
    wave_dir = ubits(bs, 325, 9);
    swell_height = ubits(bs, 334, 8) / 10.;  // m
    swell_dir = ubits(bs, 342, 9);
    swell_period = ubits(bs, 351, 6);  // s
    spare2 = ubits(bs, 357, 3);
  } else {
    // type == 1
    // PAIN IN THE ASS WMO OBS from ship
    // TODO(schwehr): double check the insanity
    x = (ubits(bs, 57, 16) / 100.) - 180;
    y = (ubits(bs, 73, 15) / 100.) - 180;

    utc_month = ubits(bs, 88, 4);
    utc_day = ubits(bs, 92, 6);
    utc_hour = ubits(bs, 98, 5);
    utc_min = ubits(bs, 102, 3) * 10;
    cog = ubits(bs, 106, 7) * 5;
    sog = ubits(bs, 113, 5) * 0.5;
    heading = ubits(bs, 118, 7) *5;  // Assume this is true degrees????
    pressure = ubits(bs, 125, 11) / 10. + 900;
    rel_pressure = ubits(bs, 136, 10) / 10. -50;
    pressure_tendency = ubits(bs, 146, 4);
    wind_dir = ubits(bs, 150, 7) * 5;
    wind_speed_ms = ubits(bs, 157, 8) * 0.5;  // m/s
    wind_dir_rel = ubits(bs, 165, 7) * 5;
    wind_speed_rel= ubits(bs, 172, 8) * 0.5;  // m/s
    wind_gust_speed = ubits(bs, 180, 8) * 0.5;  // m/s
    wind_gust_dir = ubits(bs, 188, 7) * 5;
    // 0C = 273.15 Kelvin
    // TODO(schwehr): change this to celcius
    air_temp_raw = ubits(bs, 195, 10);
    humidity =ubits(bs, 205, 7);
    water_temp_raw = ubits(bs, 212, 9);  // TODO(schwehr): change this to C
    horz_viz = pow(ubits(bs, 221, 6), 2) * 13.073;  // m
    wx[0] = ubits(bs, 227, 9);  // current
    wx[1] = ubits(bs, 236, 5);  // past 1
    wx[2] = ubits(bs, 241, 5);  // past 2
    cloud_total = ubits(bs, 246, 4) * 10;
    cloud_low = ubits(bs, 250, 4);
    cloud_low_type = ubits(bs, 254, 6);
    cloud_middle_type = ubits(bs, 260, 6);
    cloud_high_type = ubits(bs, 266, 6);
    alt_lowest_cloud_base = pow(ubits(bs, 272, 7), 2) * 0.16;
    wave_period = ubits(bs, 279, 5);  // s
    wave_height = ubits(bs, 284, 6) * 0.5;  // m
    swell_dir = ubits(bs, 290, 6) * 10;
    swell_period = ubits(bs, 296, 5);  // s
    swell_height = ubits(bs, 301, 6) * 0.5;  // m
    swell_dir_2 = ubits(bs, 307, 6) * 10;
    swell_period_2 = ubits(bs, 313, 5);  // s
    swell_height_2 = ubits(bs, 318, 6) * 0.5;  // m
    ice_thickness = ubits(bs, 324, 7) / 100.;  // network is cm, storing m
    ice_accretion = ubits(bs, 331, 3);
    ice_accretion_cause = ubits(bs, 334, 3);
    sea_ice_concentration = ubits(bs, 337, 5);
    amt_type_ice = ubits(bs, 342, 4);
    ice_situation = ubits(bs, 346, 5);
    ice_devel = ubits(bs, 351, 5);
    bearing_ice_edge = ubits(bs, 356, 4) * 45;
  }
  status = AIS_OK;
}


// IMO Circ 289 - Extended ship static and voyage-related
// See also Circ 236
Ais8_1_24::Ais8_1_24(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 24);

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

  link_id = ubits(bs, 56, 10);
  air_draught = ubits(bs, 66, 13) / 10.;  // m
  last_port = ais_str(bs, 79, 30);
  next_ports[0] = ais_str(bs, 109, 30);
  next_ports[1] = ais_str(bs, 139, 30);

  // TODO(schwehr): enum list of param types
  // 0 NA, 1 operational, 2 SNAFU, 3 no data
  for (size_t equip_num = 0; equip_num < 26; equip_num++) {
    solas_status[equip_num] = ubits(bs, 169 + 2 * equip_num, 2);
  }
  ice_class = ubits(bs, 221, 4);
  shaft_power = ubits(bs, 225, 18);  // horses
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
  status = AIS_OK;
}


// There is no 8_1_25


// See ais8_001_26.cpp


// IMO Circ 289 - Route information
// See also Circ 236
Ais8_1_27::Ais8_1_27(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 27);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  const size_t num_waypoints = (num_bits - 117) / 55;
  const size_t extra_bits = (num_bits - 117) % 55;

  if (extra_bits || num_waypoints > 16) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<997> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  link_id = ubits(bs, 56, 10);
  sender_type = ubits(bs, 66, 3);
  route_type = ubits(bs, 69, 5);
  utc_month = ubits(bs, 74, 4);
  utc_day = ubits(bs, 78, 5);
  utc_hour = ubits(bs, 83, 5);
  utc_min = ubits(bs, 88, 6);
  duration = ubits(bs, 94, 18);
  // TODO(schwehr): manage the case where num_waypoints does not match
  // const size_t num_waypoints_stated = ubits(bs, 112, 5);
  for (size_t waypoint_num = 0; waypoint_num < num_waypoints; waypoint_num++) {
    AisPoint pt;
    const size_t start = 117 + 55*waypoint_num;
    pt.x =  sbits(bs, start, 28) / 600000.;
    pt.y =  sbits(bs, start + 28, 27) / 600000.;
    waypoints.push_back(pt);
  }

  status = AIS_OK;
}

// No 8_1_28

// IMO Circ 289 - Text description
// See also Circ 236
Ais8_1_29::Ais8_1_29(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 29);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (72 > num_bits || num_bits > 1032) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<1032> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  link_id = ubits(bs, 56, 10);
  size_t text_bits = ((num_bits - 66) / 6) * 6;
  text = ais_str(bs, 66, text_bits);
  const size_t spare2_bits = num_bits - 66 - text_bits;
  if (spare2_bits) {
    const size_t start = 66 + text_bits;
    spare2 = ubits(bs, start, spare2_bits);
  } else {
    spare2 = 0;
  }

  status = AIS_OK;
}



// IMO Circ 289 - Meteorological and Hydrographic data
// See also Circ 236
Ais8_1_31::Ais8_1_31(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 31);

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

  x = sbits(bs, 56, 25) / 60000.;
  y = sbits(bs, 81, 24) / 60000.;
  position_accuracy = bs[105];
  utc_day = ubits(bs, 106, 5);
  utc_hour = ubits(bs, 111, 5);
  utc_min = ubits(bs, 116, 6);

  wind_ave = ubits(bs, 122, 7);  // kts
  wind_gust = ubits(bs, 129, 7);  // kts
  wind_dir = ubits(bs, 136, 9);
  wind_gust_dir = ubits(bs, 145, 9);
  air_temp = sbits(bs, 154, 11) / 10.;  // C
  rel_humid = ubits(bs, 165, 7);
  dew_point = sbits(bs, 172, 10)/ 10.;  // TODO(schwehr): How is this mapped?
  air_pres = ubits(bs, 182, 9);
  air_pres_trend = ubits(bs, 191, 2);

  horz_vis = ubits(bs, 193, 8) / 10.;  // NM
  water_level = ubits(bs, 201, 12) / 100. - 10;  // m
  water_level_trend = ubits(bs, 213, 2);

  surf_cur_speed = ubits(bs, 215, 8) / 10.;
  surf_cur_dir = ubits(bs, 223, 9);
  cur_speed_2 = ubits(bs, 232, 8) / 10.;  // kts
  cur_dir_2 = ubits(bs, 240, 9);
  cur_depth_2 = ubits(bs, 249, 5);  // m
  cur_speed_3 = ubits(bs, 254, 8) / 10.;  // kts
  cur_dir_3 = ubits(bs, 262, 9);
  cur_depth_3 = ubits(bs, 271, 5);  // m

  wave_height = ubits(bs, 276, 8);  // m

  wave_period = ubits(bs, 284, 6);
  wave_dir = ubits(bs, 290, 9);
  swell_height = ubits(bs, 299, 8) / 10.;
  swell_period = ubits(bs, 307, 6);
  swell_dir = ubits(bs, 313, 9);
  sea_state = ubits(bs, 322, 4);  // beaufort scale - Table 1.2
  water_temp = sbits(bs, 326, 10) / 10.;
  precip_type = ubits(bs, 336, 3);
  salinity = ubits(bs, 339, 9) / 10.;
  ice = ubits(bs, 348, 2);  // yes/no/undef/unknown
  spare2 = ubits(bs, 350, 10);

  status = AIS_OK;
}


// River Information Systems ECE-TRANS-SC3-2006-10r-RIS.pdf
// Inland ship static and voyage related data
Ais8_200_10::Ais8_200_10(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 200);
  assert(fi == 10);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  if (num_bits != 168) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<168> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  eu_id = ais_str(bs, 56, 48);
  length = ubits(bs, 104, 13) / 10.;  // m
  beam = ubits(bs, 117, 10) / 10.;  // m
  ship_type = ubits(bs, 127, 14);
  haz_cargo = ubits(bs, 141, 3);
  draught = ubits(bs, 144, 11) / 10.;  // m
  loaded = ubits(bs, 155, 2);
  speed_qual = bs[157];
  course_qual = bs[158];
  heading_qual = bs[159];
  spare2 = ubits(bs, 160, 8);

  status = AIS_OK;
}

// River Information Systems ECE-TRANS-SC3-2006-10r-RIS.pdf
Ais8_200_23::Ais8_200_23(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 200);
  assert(fi == 23);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 256) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<256> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  utc_year_start = ubits(bs, 56, 9);
  utc_month_start = ubits(bs, 65, 4);
  utc_day_start = ubits(bs, 69, 4);  // ERROR: not enough bits to cover 1-31
  utc_year_end = ubits(bs, 73, 9);
  utc_month_end = ubits(bs, 82, 4);
  utc_day_end = ubits(bs, 86, 4);  // ERROR: not enough bits to cover 1-31

  utc_hour_start = ubits(bs, 90, 5);
  utc_min_start = ubits(bs, 95, 6);
  utc_hour_end = ubits(bs, 101, 5);
  utc_min_end = ubits(bs, 106, 6);

  x1 = sbits(bs, 112, 28) / 600000.;
  y1 = sbits(bs, 140, 27) / 600000.;
  x2 = sbits(bs, 167, 28) / 600000.;
  y2 = sbits(bs, 195, 27) / 600000.;

  type = ubits(bs, 222, 4);
  min = ubits(bs, 226, 9);
  max = ubits(bs, 235, 9);
  classification = ubits(bs, 244, 2);
  wind_dir = ubits(bs, 246, 4);
  spare2 = ubits(bs, 250, 6);

  status = AIS_OK;
}


// River Information Systems ECE-TRANS-SC3-2006-10r-RIS.pdf
Ais8_200_24::Ais8_200_24(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 200);
  assert(fi == 24);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 168) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<168> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  ais_str(bs, 56, 12);
  for (size_t i = 0; i < 4; i++) {
    size_t start = 68 + 25*i;
    guage_ids[i] = ubits(bs, start, 11);
    const int sign = bs[start + 11] ? 1 : -1;  // 0 negative, 1 pos
    // ERROR: the spec has a bit listing mistake
    levels[i] = sign * ubits(bs, start + 12, 13);
  }
  status = AIS_OK;
}

// River Information Systems ECE-TRANS-SC3-2006-10r-RIS.pdf
Ais8_200_40::Ais8_200_40(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 200);
  assert(fi == 40);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (num_bits != 168) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<168> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  x = sbits(bs, 56, 28) / 600000.;
  y = sbits(bs, 84, 27) / 600000.;
  form = ubits(bs, 111, 4);
  dir = ubits(bs, 115, 9);  // degrees
  stream_dir = ubits(bs, 124, 3);
  status_raw = ubits(bs, 127, 30);
  // TODO(schwehr): status[ ] = bite me;
  spare2 = ubits(bs, 157, 11);

  status = AIS_OK;
}


// River Information Systems ECE-TRANS-SC3-2006-10r-RIS.pdf
Ais8_200_55::Ais8_200_55(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 200);
  assert(fi == 55);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  // People might get smart and leave out the 51 spare bits
  // TODO(schwehr): do we have any cases of that?
  if (num_bits != 88 && num_bits != 136 && num_bits != 168) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<168> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  crew = ubits(bs, 56, 8);
  passengers = ubits(bs, 64, 13);
  yet_more_personnel = ubits(bs, 77, 8);
  if (88 == num_bits) {
    spare2[0] = ubits(bs, 85, 3);
    spare2[1] = 0;
    spare2[2] = 0;
  } else if (136 == num_bits) {
    // as in the spec - maybe?
    spare2[0] = ubits(bs, 85, 32);
    spare2[1] = ubits(bs, 117, 32);
    spare2[2] = ubits(bs, 149, 19);
  } else {
    spare2[0] = ubits(bs, 85, 32);
    spare2[1] = ubits(bs, 117, 19);
    spare2[2] = 0;
  }

  status = AIS_OK;
}
