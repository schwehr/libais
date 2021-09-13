// 8:367:33 Defined by EM Version 3-23MAR15.pdf.
// The USCG Environmental Message

#include "ais.h"

namespace libais {

Ais8_367_33_Location::Ais8_367_33_Location(const AisBitset &bits,
                                           const size_t offset) {
  version = bits.ToUnsignedInt(offset, 6);
  position = bits.ToAisPoint(offset + 6, 55);

  precision = bits.ToUnsignedInt(offset + 61, 3);
  altitude = bits.ToUnsignedInt(offset + 64, 12) / 10.;  // meters
  owner = bits.ToUnsignedInt(offset + 76, 4);
  timeout = bits.ToUnsignedInt(offset + 80, 3);
  spare2 = bits.ToUnsignedInt(offset + 83, 2);
}

Ais8_367_33_Station::Ais8_367_33_Station(const AisBitset &bits,
                                     const size_t offset) {
  name = bits.ToString(offset, 84);
  spare2 = bits.ToUnsignedInt(offset + 84, 1);
}

Ais8_367_33_Wind::Ais8_367_33_Wind(const AisBitset &bits,
                                   const size_t offset) {
  wind_speed = bits.ToUnsignedInt(offset, 7);
  wind_gust  = bits.ToUnsignedInt(offset + 7, 7);  // knots
  wind_dir = bits.ToUnsignedInt(offset + 14, 9);
  wind_gust_dir = bits.ToUnsignedInt(offset + 23, 9);
  sensor_type = bits.ToUnsignedInt(offset + 32, 3);
  wind_forecast = bits.ToUnsignedInt(offset + 35, 7);
  wind_gust_forecast = bits.ToUnsignedInt(offset + 42, 7);  // knots
  wind_dir_forecast = bits.ToUnsignedInt(offset + 49, 9);
  utc_day_forecast = bits.ToUnsignedInt(offset + 58, 5);
  utc_hour_forecast = bits.ToUnsignedInt(offset + 63, 5);
  utc_min_forecast = bits.ToUnsignedInt(offset + 68, 6);
  duration = bits.ToUnsignedInt(offset + 74, 8);
  spare2 = bits.ToUnsignedInt(offset + 82, 3);
}

Ais8_367_33_WaterLevel::Ais8_367_33_WaterLevel(const AisBitset &bits,
                                               const size_t offset) {
  type = bits[offset];
  level = bits.ToInt(offset + 1, 16);
  trend = bits.ToUnsignedInt(offset + 17, 2);
  vdatum = bits.ToUnsignedInt(offset + 19, 5);
  sensor_type = bits.ToUnsignedInt(offset + 24, 3);
  forecast_type = bits[offset + 27];
  level_forecast = bits.ToInt(offset + 28, 16);
  utc_day_forecast = bits.ToUnsignedInt(offset + 44, 5);
  utc_hour_forecast = bits.ToUnsignedInt(offset + 49, 5);
  utc_min_forecast = bits.ToUnsignedInt(offset + 54, 6);
  duration = bits.ToUnsignedInt(offset + 60, 8);
  spare2 = bits.ToUnsignedInt(offset + 68, 17);
}

Ais8_367_33_Curr2D::Ais8_367_33_Curr2D(const AisBitset &bits,
                                       const size_t offset) {
  for (size_t idx = 0; idx < 3; idx++) {
    size_t start = offset + idx * 26;
    currents[idx].speed = bits.ToUnsignedInt(start, 8) / 10.;
    currents[idx].dir = bits.ToUnsignedInt(start + 8, 9);
    currents[idx].depth = bits.ToUnsignedInt(start + 17, 9);
  }
  type = bits.ToUnsignedInt(offset + 78, 3);
  spare2 = bits.ToUnsignedInt(offset + 81, 4);
}

Ais8_367_33_Curr3D::Ais8_367_33_Curr3D(const AisBitset &bits,
                                       const size_t offset) {
  for (size_t idx = 0; idx < 2; idx++) {
    size_t start = offset + idx * 36;
    currents[idx].north = bits.ToUnsignedInt(start, 9) / 10.;
    currents[idx].east = bits.ToUnsignedInt(start + 9, 9) / 10.;
    currents[idx].up = bits.ToUnsignedInt(start + 18, 9) / 10.;
    currents[idx].depth = bits.ToUnsignedInt(start + 27, 9);
  }
  type = bits.ToUnsignedInt(offset + 72, 3);
  spare2 = bits.ToUnsignedInt(offset + 75, 10);
}

Ais8_367_33_HorzFlow::Ais8_367_33_HorzFlow(const AisBitset &bits,
                                           const size_t offset) {
  bearing = bits.ToUnsignedInt(offset, 9);
  for (size_t idx = 0; idx < 2; idx++) {
    size_t start = offset + 9 + (idx * 35);
    currents[idx].dist = bits.ToUnsignedInt(start, 9);
    currents[idx].speed = bits.ToUnsignedInt(start + 9, 8) / 10.;
    currents[idx].dir = bits.ToUnsignedInt(start + 17, 9);
    currents[idx].level = bits.ToUnsignedInt(start + 26, 9);
  }
  type = bits.ToUnsignedInt(offset + 79, 3);
  spare2 = bits.ToUnsignedInt(offset + 82, 3);
}

Ais8_367_33_SeaState::Ais8_367_33_SeaState(const AisBitset &bits,
                                           const size_t offset) {
  swell_height = bits.ToUnsignedInt(offset, 8) / 10.;
  swell_period = bits.ToUnsignedInt(offset + 8, 6);
  swell_dir = bits.ToUnsignedInt(offset + 14, 9);
  sea_state = bits.ToUnsignedInt(offset + 23, 4);
  swell_sensor_type = bits.ToUnsignedInt(offset + 27, 3);
  water_temp = bits.ToInt(offset + 30, 10) / 10.;
  water_temp_depth = bits.ToUnsignedInt(offset + 40, 7) / 10.;
  water_sensor_type = bits.ToUnsignedInt(offset + 47, 3);
  wave_height = bits.ToUnsignedInt(offset + 50, 8) / 10.;
  wave_period = bits.ToUnsignedInt(offset + 58, 6);
  wave_dir = bits.ToUnsignedInt(offset + 64, 9);
  wave_sensor_type = bits.ToUnsignedInt(offset + 73, 3);
  salinity = bits.ToUnsignedInt(offset + 76, 9) / 10.;
}


Ais8_367_33_Salinity::Ais8_367_33_Salinity(const AisBitset &bits,
                                           const size_t offset) {
  water_temp = bits.ToUnsignedInt(offset, 10) / 10. - 10;
  conductivity = bits.ToUnsignedInt(offset + 10, 10) / 100.;
  pressure = bits.ToUnsignedInt(offset + 20, 16) / 10.;
  salinity = bits.ToUnsignedInt(offset + 36, 9) / 10.;
  salinity_type = bits.ToUnsignedInt(offset + 45, 2);
  sensor_type = bits.ToUnsignedInt(offset + 47, 3);
  spare2[0] = bits.ToUnsignedInt(offset + 50, 32);
  spare2[1] = bits.ToUnsignedInt(offset + 82, 3);
}

Ais8_367_33_Wx::Ais8_367_33_Wx(const AisBitset &bits,
                               const size_t offset) {
  air_temp = bits.ToInt(offset, 11) / 10.;
  air_temp_sensor_type = bits.ToUnsignedInt(offset + 11, 3);
  precip = bits.ToUnsignedInt(offset + 14, 2);
  horz_vis = bits.ToUnsignedInt(offset + 16, 8) / 10.;
  dew_point = (bits.ToUnsignedInt(offset + 24, 10) / 10.) - 20.0;
  dew_point_type = bits.ToUnsignedInt(offset + 34, 3);
  air_pressure = bits.ToUnsignedInt(offset + 37, 9) + 800;  // hPa.
  air_pressure_trend = bits.ToUnsignedInt(offset + 46, 2);
  air_pressor_type = bits.ToUnsignedInt(offset + 48, 3);
  salinity = bits.ToUnsignedInt(offset + 51, 9) / 10.;
  spare2 = bits.ToUnsignedInt(offset + 60, 25);
}

Ais8_367_33_AirGap::Ais8_367_33_AirGap(const AisBitset &bits,
                                       const size_t offset) {
  draught = bits.ToUnsignedInt(offset, 13) / 100.;
  gap = bits.ToUnsignedInt(offset + 13, 13) / 10.;
  trend = bits.ToUnsignedInt(offset + 26, 2);
  forecast_gap = bits.ToUnsignedInt(offset + 28, 13) / 10.;
  utc_day_forecast = bits.ToUnsignedInt(offset + 41, 5);
  utc_hour_forecast = bits.ToUnsignedInt(offset + 46, 5);
  utc_min_forecast = bits.ToUnsignedInt(offset + 51, 6);
  type = bits.ToUnsignedInt(offset + 57, 3);
  spare2 = bits.ToUnsignedInt(offset + 60, 25);
}

Ais8_367_33_Wind_V2::Ais8_367_33_Wind_V2(const AisBitset &bits,
                                         const size_t offset) {
  wind_speed = bits.ToUnsignedInt(offset, 7);
  wind_gust = bits.ToUnsignedInt(offset + 7, 7);
  wind_dir = bits.ToUnsignedInt(offset + 14, 7);
  averaging_time = bits.ToUnsignedInt(offset + 23, 6);
  sensor_type = bits.ToUnsignedInt(offset + 29, 3);
  wind_speed_forecast = bits.ToUnsignedInt(offset + 32, 7);
  wind_gust_forecast = bits.ToUnsignedInt(offset + 39, 7);
  wind_dir_forecast = bits.ToUnsignedInt(offset + 46, 9);
  utc_hour_forecast = bits.ToUnsignedInt(offset + 55, 5);
  utc_min_forecast = bits.ToUnsignedInt(offset + 60, 6);
  duration = bits.ToUnsignedInt(offset + 66, 8);
  spare2 = bits.ToUnsignedInt(offset + 74, 11);
}


Ais8_367_33_SensorReport*
ais8_367_33_sensor_report_factory(const AisBitset &bits,
                                  const size_t offset) {
  const Ais8_367_33_SensorEnum rpt_type =
      (Ais8_367_33_SensorEnum)bits.ToUnsignedInt(offset, 4);

  // WARNING: out of order decoding
  // Only get the report header if we can decode the type
  const size_t rpt_start = offset + 27;  // skip tp after site_id
  bits.SeekTo(rpt_start);
  Ais8_367_33_SensorReport *rpt = nullptr;
  switch (rpt_type) {
  case AIS8_367_33_SENSOR_LOCATION:
    rpt = new Ais8_367_33_Location(bits, rpt_start);
    break;
  case AIS8_367_33_SENSOR_STATION:
    rpt = new Ais8_367_33_Station(bits, rpt_start);
    break;
  case AIS8_367_33_SENSOR_WIND:
    rpt = new Ais8_367_33_Wind(bits, rpt_start);
    break;
  case AIS8_367_33_SENSOR_WATER_LEVEL:
    rpt = new Ais8_367_33_WaterLevel(bits, rpt_start);
    break;
  case AIS8_367_33_SENSOR_CURR_2D:
    rpt = new Ais8_367_33_Curr2D(bits, rpt_start);
    break;
  case AIS8_367_33_SENSOR_CURR_3D:
    rpt = new Ais8_367_33_Curr3D(bits, rpt_start);
    break;
  case AIS8_367_33_SENSOR_HORZ_FLOW:
    rpt = new Ais8_367_33_HorzFlow(bits, rpt_start);
    break;
  case AIS8_367_33_SENSOR_SEA_STATE:
    rpt = new Ais8_367_33_SeaState(bits, rpt_start);
    break;
  case AIS8_367_33_SENSOR_SALINITY:
    rpt = new Ais8_367_33_Salinity(bits, rpt_start);
    break;
  case AIS8_367_33_SENSOR_WX:
    rpt = new Ais8_367_33_Wx(bits, rpt_start);
    break;
  case AIS8_367_33_SENSOR_AIR_GAP:
    rpt = new Ais8_367_33_AirGap(bits, rpt_start);
    break;
  case AIS8_367_33_SENSOR_WIND_REPORT_2:
    rpt = new Ais8_367_33_Wind_V2(bits, rpt_start);
  break;
  // Leave rpt == 0 to indicate error
  case AIS8_367_33_SENSOR_RESERVED_12: break;
  case AIS8_367_33_SENSOR_RESERVED_13: break;
  case AIS8_367_33_SENSOR_RESERVED_14: break;
  case AIS8_367_33_SENSOR_RESERVED_15: break;
  default:
    {}  // Leave rpt == 0 to indicate error
  }

  if (!rpt) {
    return rpt;
  }

  rpt->report_type = rpt_type;
  bits.SeekTo(offset + 4);
  rpt->utc_day = bits.ToUnsignedInt(offset + 4, 5);
  rpt->utc_hr = bits.ToUnsignedInt(offset + 9, 5);
  rpt->utc_min = bits.ToUnsignedInt(offset + 14, 6);
  rpt->site_id = bits.ToUnsignedInt(offset + 20, 7);
  return rpt;
}

Ais8_367_33::Ais8_367_33(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  assert(dac == 33);
  assert(fi == 33);

  if (!CheckStatus()) {
    return;
  }
  if (168 > num_bits || num_bits > 952) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  const size_t num_sensor_reports = (num_bits - 56) / AIS8_367_33_REPORT_SIZE;

  if (num_sensor_reports > 8) {
    return;
  }

  for (size_t report_idx = 0; report_idx < num_sensor_reports; report_idx++) {
    const size_t start = 56 + report_idx * AIS8_367_33_REPORT_SIZE;
    bits.SeekTo(start);
    Ais8_367_33_SensorReport *sensor =
        ais8_367_33_sensor_report_factory(bits, start);
    if (sensor) {
      reports.push_back(sensor);
    } else {
      status = AIS_ERR_BAD_SUB_SUB_MSG;
      return;
    }
  }

  // NOTE: Enable this assert after fixing the message.
  // assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

// TODO(schwehr): Use unique_ptr to manage memory.
Ais8_367_33::~Ais8_367_33() {
  for (size_t i = 0; i < reports.size(); i++) {
    delete reports[i];
    reports[i] = nullptr;
  }
}

}  // namespace libais
