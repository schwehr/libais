// The USCG RTCM SC 121 Environmental Sensor Message tested in
// Tampa, FL from 200[78] util ?

// http://www.dtic.mil/cgi-bin/GetTRDoc?AD=ADA504755
// Phase I Summary Report on AIS Transmit Project (Environmental Message)

#include "ais.h"

Ais8_1_26_Location::Ais8_1_26_Location(const bitset<AIS8_MAX_BITS> &bs,
                                       const size_t offset) {
  x = sbits(bs, offset, 28) / 600000.;
  y = sbits(bs, offset + 28, 27) / 600000.;
  z = ubits(bs, offset + 55, 11) / 10.;
  owner = ubits(bs, offset + 66, 4);
  timeout = ubits(bs, offset + 70, 3);
  spare = ubits(bs, offset + 73, 12);
}

Ais8_1_26_Station::Ais8_1_26_Station(const bitset<AIS8_MAX_BITS> &bs,
                                     const size_t offset) {
  name = ais_str(bs, offset, 84);
  spare = ubits(bs, offset + 84, 1);
}

Ais8_1_26_Wind::Ais8_1_26_Wind(const bitset<AIS8_MAX_BITS> &bs,
                               const size_t offset) {
  wind_speed = ubits(bs, offset, 7);
  wind_gust  = ubits(bs, offset + 7, 7);  // knots
  wind_dir = ubits(bs, offset + 14, 9);
  wind_gust_dir = ubits(bs, offset + 23, 9);
  sensor_type = ubits(bs, offset + 32, 3);
  wind_forcast = ubits(bs, offset + 35, 7);
  wind_gust_forcast = ubits(bs, offset + 42, 7);  // knots
  wind_dir_forcast = ubits(bs, offset + 49, 9);
  utc_day_forcast = ubits(bs, offset + 58, 5);
  utc_hour_forcast = ubits(bs, offset + 63, 5);
  utc_min_forcast = ubits(bs, offset + 68, 6);
  duration = ubits(bs, offset + 74, 8);
  spare = ubits(bs, offset + 82, 3);
}

Ais8_1_26_WaterLevel::Ais8_1_26_WaterLevel(const bitset<AIS8_MAX_BITS> &bs,
                                           const size_t offset) {
  type = bs[offset];
  level = sbits(bs, offset + 1, 16) / 100.;
  trend = ubits(bs, offset + 17, 2);
  vdatum = ubits(bs, offset + 19, 5);
  sensor_type = ubits(bs, offset + 24, 3);
  forcast_type = bs[offset + 27];
  level_forcast = sbits(bs, offset + 28, 16) / 100.;
  utc_day_forcast = ubits(bs, offset + 44, 5);
  utc_hour_forcast = ubits(bs, offset + 49, 5);
  utc_min_forcast = ubits(bs, offset + 54, 6);
  duration = ubits(bs, offset + 60, 8);
  spare = ubits(bs, offset + 68, 17);
}

Ais8_1_26_Curr2D::Ais8_1_26_Curr2D(const bitset<AIS8_MAX_BITS> &bs,
                                   const size_t offset) {
  for (size_t idx = 0; idx < 3; idx++) {
    size_t start = offset + idx * 26;
    currents[idx].speed = ubits(bs, start, 8) / 10.;
    currents[idx].dir = ubits(bs, start + 8, 9);
    currents[idx].depth = ubits(bs, start + 17, 9);
  }
  type = ubits(bs, offset + 78, 3);
  spare = ubits(bs, offset + 81, 4);
}

Ais8_1_26_Curr3D::Ais8_1_26_Curr3D(const bitset<AIS8_MAX_BITS> &bs,
                                   const size_t offset) {
  for (size_t idx = 0; idx < 2; idx++) {
    size_t start = offset + idx * 33;
    currents[idx].north = ubits(bs, start, 8) / 10.;
    currents[idx].east = ubits(bs, start + 8, 8) / 10.;
    currents[idx].up = ubits(bs, start + 16, 8) / 10.;
    currents[idx].depth = ubits(bs, start + 24, 9);
  }
  type = ubits(bs, offset + 66, 3);
  spare = ubits(bs, offset + 69, 16);
}

Ais8_1_26_HorzFlow::Ais8_1_26_HorzFlow(const bitset<AIS8_MAX_BITS> &bs,
                                       const size_t offset) {
  for (size_t idx = 0; idx < 2; idx++) {
    size_t start = offset + idx * 42;
    currents[idx].bearing = ubits(bs, start, 9);
    currents[idx].dist = ubits(bs, start + 9, 7);
    currents[idx].speed = ubits(bs, start + 16, 8) / 10.;
    currents[idx].dir = ubits(bs, start + 24, 9);
    currents[idx].level = ubits(bs, start + 33, 9);
  }
  spare = bs[offset + 84];
}

Ais8_1_26_SeaState::Ais8_1_26_SeaState(const bitset<AIS8_MAX_BITS> &bs,
                                       const size_t offset) {
  swell_height = ubits(bs, offset, 8) / 10.;
  swell_period = ubits(bs, offset + 8, 6);
  swell_dir = ubits(bs, offset + 14, 9);
  sea_state = ubits(bs, offset + 23, 4);
  swell_sensor_type = ubits(bs, offset + 27, 3);
  water_temp = sbits(bs, offset + 30, 10) / 10.;
  water_temp_depth = ubits(bs, offset + 40, 7) / 10.;
  water_sensor_type = ubits(bs, offset + 47, 3);
  wave_height = ubits(bs, offset + 50, 8) / 10.;
  wave_period = ubits(bs, offset + 58, 6);
  wave_dir = ubits(bs, offset + 64, 9);
  wave_sensor_type = ubits(bs, offset + 73, 3);
  salinity = ubits(bs, offset + 76, 9) / 10.;
}

Ais8_1_26_Salinity::Ais8_1_26_Salinity(const bitset<AIS8_MAX_BITS> &bs,
                                       const size_t offset) {
  water_temp = ubits(bs, offset, 10) / 10. - 10;
  conductivity = ubits(bs, offset + 10, 10) / 100.;
  pressure = ubits(bs, offset + 20, 16) / 10.;
  salinity = ubits(bs, offset + 36, 9) / 10.;
  salinity_type = ubits(bs, offset + 45, 2);
  sensor_type = ubits(bs, offset + 47, 3);
  spare[0] = ubits(bs, offset + 50, 32);
  spare[1] = ubits(bs, offset + 82, 3);
}

Ais8_1_26_Wx::Ais8_1_26_Wx(const bitset<AIS8_MAX_BITS> &bs,
                           const size_t offset) {
  air_temp = sbits(bs, offset, 11) / 10.;
  air_temp_sensor_type = ubits(bs, offset + 11, 3);
  precip = ubits(bs, offset + 14, 2);
  horz_vis = ubits(bs, offset + 16, 8) / 10.;
  dew_point = sbits(bs, offset + 24, 10) / 10.;
  dew_point_type = ubits(bs, offset + 34, 3);
  air_pressure = ubits(bs, offset + 37, 9);
  air_pressure_trend = ubits(bs, offset + 46, 2);
  air_pressor_type = ubits(bs, offset + 48, 3);
  salinity = ubits(bs, offset + 51, 9) / 10.;
  spare = ubits(bs, offset + 60, 25);
}

Ais8_1_26_AirDraught::Ais8_1_26_AirDraught(const bitset<AIS8_MAX_BITS> &bs,
                                           const size_t offset) {
  draught = ubits(bs, offset, 13) / 100.;
  gap = ubits(bs, offset + 13, 13) / 10.;
  trend = ubits(bs, offset + 26, 2);
  forcast_gap = ubits(bs, offset + 28, 13) / 10.;
  utc_day_forcast = ubits(bs, offset + 41, 5);
  utc_hour_forcast = ubits(bs, offset + 46, 5);
  utc_min_forcast = ubits(bs, offset + 51, 6);
  spare = ubits(bs, offset + 57, 28);
}

Ais8_1_26_SensorReport*
ais8_1_26_sensor_report_factory(const bitset<AIS8_MAX_BITS> &bs,
                                const size_t offset) {
  const Ais8_1_26_SensorEnum rpt_type =
      (Ais8_1_26_SensorEnum)ubits(bs, offset, 4);

  // WARNING: out of order decoding
  // Only get the report header if we can decode the type
  const size_t rpt_start = offset + 27;  // skip tp after site_id

  Ais8_1_26_SensorReport *rpt = NULL;
  switch (rpt_type) {
  case AIS8_1_26_SENSOR_LOCATION:
    rpt = new Ais8_1_26_Location(bs, rpt_start);
    break;
  case AIS8_1_26_SENSOR_STATION:
    rpt = new Ais8_1_26_Station(bs, rpt_start);
    break;
  case AIS8_1_26_SENSOR_WIND:
    rpt = new Ais8_1_26_Wind(bs, rpt_start);
    break;
  case AIS8_1_26_SENSOR_WATER_LEVEL:
    rpt = new Ais8_1_26_WaterLevel(bs, rpt_start);
    break;
  case AIS8_1_26_SENSOR_CURR_2D:
    rpt = new Ais8_1_26_Curr2D(bs, rpt_start);
    break;
  case AIS8_1_26_SENSOR_CURR_3D:
    rpt = new Ais8_1_26_Curr3D(bs, rpt_start);
    break;
  case AIS8_1_26_SENSOR_HORZ_FLOW:
    rpt = new Ais8_1_26_HorzFlow(bs, rpt_start);
    break;
  case AIS8_1_26_SENSOR_SEA_STATE:
    rpt = new Ais8_1_26_SeaState(bs, rpt_start);
    break;
  case AIS8_1_26_SENSOR_SALINITY:
    rpt = new Ais8_1_26_Salinity(bs, rpt_start);
    break;
  case AIS8_1_26_SENSOR_WX:
    rpt = new Ais8_1_26_Wx(bs, rpt_start);
    break;
  case AIS8_1_26_SENSOR_AIR_DRAUGHT:
    rpt = new Ais8_1_26_AirDraught(bs, rpt_start);
    break;
  case AIS8_1_26_SENSOR_RESERVED_11: break;  // Leave rpt == 0 to indicate error
  case AIS8_1_26_SENSOR_RESERVED_12: break;
  case AIS8_1_26_SENSOR_RESERVED_13: break;
  case AIS8_1_26_SENSOR_RESERVED_14: break;
  case AIS8_1_26_SENSOR_RESERVED_15: break;
  default:
    {}  // Leave rpt == 0 to indicate error
  }

  if (!rpt)
    return rpt;

  rpt->report_type = rpt_type;
  rpt->utc_day = ubits(bs, offset + 4, 5);
  rpt->utc_hr = ubits(bs, offset + 9, 5);
  rpt->utc_min = ubits(bs, offset + 14, 6);
  rpt->site_id = ubits(bs, offset + 20, 7);
  return rpt;
}

Ais8_1_26::Ais8_1_26(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 26);

  const int num_bits = strlen(nmea_payload) * 6 - pad;
  if (168 > num_bits || num_bits > 1098) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<MAX_BITS> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  const size_t num_sensor_reports = (num_bits - 56) / AIS8_1_26_REPORT_SIZE;

  // TODO(schwehr): what to do about extra data in sensor report msg 8_1_26?
  // if ((num_bits - 56) % AIS8_1_26_REPORT_SIZE)

  for (size_t report_idx = 0; report_idx < num_sensor_reports; report_idx++) {
    const size_t start = 56 + report_idx * AIS8_1_26_REPORT_SIZE;
    Ais8_1_26_SensorReport *sensor = ais8_1_26_sensor_report_factory(bs, start);
    if (sensor) {
        reports.push_back(sensor);
    } else {
      status = AIS_ERR_BAD_SUB_SUB_MSG;
    }
  }
  if (AIS_UNINITIALIZED == status)
    status = AIS_OK;
}

Ais8_1_26::~Ais8_1_26() {
  for (size_t i = 0; i < reports.size(); i++) {
    delete reports[i];
    reports[i] = NULL;
  }
}
