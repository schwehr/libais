// 8:367:22 Defined by an email from Greg Johnson representing the
// USCG, Fall 2012.  Breaks from the RTCM and IMO Circular 289.
// "Area Notice Message Release Version: 1" 13 Aug 2012
//
// http://www.e-navigation.nl/content/geographic-notice
// http://www.e-navigation.nl/sites/default/files/asm_files/GN%20Release%20Version%201b.pdf
//
// 8:367:23
// https://www.e-navigation.nl/content/satellite-ship-weather
// 8:367:24
// https://www.e-navigation.nl/content/satellite-ship-weather-small
// 8:367:25
// https://www.e-navigation.nl/content/satellite-ship-weather-tiny

#include <cassert>
#include <cmath>
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include "ais.h"

namespace libais {

const size_t SUB_AREA_BITS = 96;

static int scale_multipliers[4] = {1, 10, 100, 1000};

Ais8_367_22_Circle::Ais8_367_22_Circle(
    const AisBitset &bits, const size_t offset)
    : precision(0), radius_m(0), spare(0) {
  const int scale_factor = bits.ToUnsignedInt(offset, 2);
  position = bits.ToAisPoint(offset + 2, 55);
  precision = bits.ToUnsignedInt(offset + 57, 3);
  radius_m =
      bits.ToUnsignedInt(offset + 60, 12) * scale_multipliers[scale_factor];
  spare = bits.ToUnsignedInt(offset + 72, 21);
}

Ais8_367_22_Rect::Ais8_367_22_Rect(const AisBitset &bits, const size_t offset)
    : precision(0), e_dim_m(0), n_dim_m(0), orient_deg(0), spare(0) {
  const int scale_factor = bits.ToUnsignedInt(offset, 2);
  position = bits.ToAisPoint(offset + 2, 55);
  precision = bits.ToUnsignedInt(offset + 57, 3);
  e_dim_m =
      bits.ToUnsignedInt(offset + 60, 8) * scale_multipliers[scale_factor];
  n_dim_m =
      bits.ToUnsignedInt(offset + 68, 8) * scale_multipliers[scale_factor];
  orient_deg = bits.ToUnsignedInt(offset + 76, 9);
  spare = bits.ToUnsignedInt(offset + 85, 8);
}

Ais8_367_22_Sector::Ais8_367_22_Sector(
    const AisBitset &bits, const size_t offset)
    : precision(0),
      radius_m(0),
      left_bound_deg(0),
      right_bound_deg(0),
      spare(0) {
  const int scale_factor = bits.ToUnsignedInt(offset, 2);
  position = bits.ToAisPoint(offset + 2, 55);
  precision = bits.ToUnsignedInt(offset + 57, 3);
  radius_m =
      bits.ToUnsignedInt(offset + 60, 12) * scale_multipliers[scale_factor];
  left_bound_deg = bits.ToUnsignedInt(offset + 72, 9);
  right_bound_deg = bits.ToUnsignedInt(offset + 81, 9);
  spare = bits.ToUnsignedInt(offset + 90, 3);
}

// Polyline or polygon.
Ais8_367_22_Poly::Ais8_367_22_Poly(const AisBitset &bits, const size_t offset,
                                   Ais8_366_22_AreaShapeEnum area_shape)
    : shape(area_shape), precision(0), spare(0) {
  const int scale_factor = bits.ToUnsignedInt(offset, 2);
  size_t poly_offset = offset + 2;
  for (size_t i = 0; i < 4; i++) {
    const int angle = bits.ToUnsignedInt(poly_offset, 10);
    poly_offset += 10;
    const int dist = bits.ToUnsignedInt(poly_offset, 11) *
                     scale_multipliers[scale_factor];
    poly_offset += 11;
    if (dist == 0) {
      break;
    }
    angles.push_back(angle);
    dists_m.push_back(dist);
  }
  spare = bits.ToUnsignedInt(offset + 86, 7);
}

Ais8_367_22_Text::Ais8_367_22_Text(const AisBitset &bits, const size_t offset) {
  text = std::string(bits.ToString(offset, 90));
  spare = bits.ToUnsignedInt(offset + 90, 3);
}

std::unique_ptr<Ais8_367_22_SubArea>
ais8_367_22_subarea_factory(const AisBitset &bits,
                            const size_t offset) {
  const auto area_shape =
      static_cast<Ais8_366_22_AreaShapeEnum>(bits.ToUnsignedInt(offset, 3));

  switch (area_shape) {
    case AIS8_366_22_SHAPE_CIRCLE:
      return std::unique_ptr<Ais8_367_22_SubArea>(new Ais8_367_22_Circle(bits, offset + 3));
    case AIS8_366_22_SHAPE_RECT:
      return std::unique_ptr<Ais8_367_22_SubArea>(new Ais8_367_22_Rect(bits, offset + 3));
    case AIS8_366_22_SHAPE_SECTOR:
      return std::unique_ptr<Ais8_367_22_SubArea>(new Ais8_367_22_Sector(bits, offset + 3));
    case AIS8_366_22_SHAPE_POLYLINE:  // FALLTHROUGH
    case AIS8_366_22_SHAPE_POLYGON:
      return std::unique_ptr<Ais8_367_22_SubArea>(new Ais8_367_22_Poly(bits, offset + 3, area_shape));
    case AIS8_366_22_SHAPE_TEXT:
      return std::unique_ptr<Ais8_367_22_SubArea>(new Ais8_367_22_Text(bits, offset + 3));
    case AIS8_366_22_SHAPE_RESERVED_6:  // FALLTHROUGH
    case AIS8_366_22_SHAPE_RESERVED_7:  // FALLTHROUGH
      // Leave area as 0 to indicate error.
      break;
    case AIS8_366_22_SHAPE_ERROR:
      break;
    default:
      assert(false);
  }
  return nullptr;
}

Ais8_367_22::Ais8_367_22(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad), version(0), link_id(0), notice_type(0),
      month(0), day(0), hour(0), minute(0), duration_minutes(0), spare2(0) {
  assert(dac == 367);
  assert(fi == 22);

  if (!CheckStatus()) {
    return;
  }
  if (num_bits < 216 || num_bits > 1016) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }
  // Is there more than spare bits after the last submessage?
  if ((num_bits - 120) % SUB_AREA_BITS >= 6) {
    // More than just spare.
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(56);
  version = bits.ToUnsignedInt(56, 6);
  link_id = bits.ToUnsignedInt(62, 10);
  notice_type = bits.ToUnsignedInt(72, 7);
  month = bits.ToUnsignedInt(79, 4);
  day = bits.ToUnsignedInt(83, 5);
  hour = bits.ToUnsignedInt(88, 5);
  minute = bits.ToUnsignedInt(93, 6);
  duration_minutes = bits.ToUnsignedInt(99, 18);
  spare2 = bits.ToUnsignedInt(117, 3);

  const int num_sub_areas = static_cast<int>(
      floor((num_bits - 120) / static_cast<float>(SUB_AREA_BITS)));

  for (int area_idx = 0; area_idx < num_sub_areas; area_idx++) {
    const size_t start = 120 + area_idx * SUB_AREA_BITS;
    std::unique_ptr<Ais8_367_22_SubArea> area = ais8_367_22_subarea_factory(bits, start);
    if (area != nullptr) {
      sub_areas.push_back(std::move(area));
    } else {
      status = AIS_ERR_BAD_SUB_SUB_MSG;
      return;
    }
  }

  // TODO(schwehr): Save the spare bits at the end of the message.
  assert(bits.GetRemaining() < 6);
  status = AIS_OK;
}

// SSW FI23 Satellite Ship Weather 1-Slot Version
Ais8_367_23::Ais8_367_23(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  assert(dac == 367);
  assert(fi == 23);

  if (!CheckStatus()) {
    return;
  }

  if (num_bits != 168) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(56);

  version = bits.ToUnsignedInt(56, 3);
  utc_day = bits.ToUnsignedInt(59, 5);
  utc_hour = bits.ToUnsignedInt(64, 5);
  utc_min = bits.ToUnsignedInt(69, 6);
  position = bits.ToAisPoint(75, 49);
  pressure = bits.ToUnsignedInt(124, 9) + 799;  // hPa
  air_temp_raw = bits.ToInt(133, 11);
  air_temp = air_temp_raw / 10.;  // C
  wind_speed = bits.ToUnsignedInt(144, 7);  // ave knots
  wind_gust = bits.ToUnsignedInt(151, 7);  // ave knots
  wind_dir = bits.ToUnsignedInt(158, 9);

  spare2 = bits.ToUnsignedInt(167, 1);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

std::ostream& operator<< (std::ostream &o, const Ais8_367_23 &msg) {
  return o << msg.mmsi << ": " << msg.version << ": " << msg.utc_day << ": " << msg.utc_hour << ": " << msg.utc_min << ": " << msg.position << ": " << msg.pressure << ": " << msg.air_temp << ": " << msg.wind_speed << ": " << msg.wind_gust << ": " << msg.wind_dir;
}

// SSW FI24 Satellite Ship Weather Small - Less than 1-Slot Version
Ais8_367_24::Ais8_367_24(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  assert(dac == 367);
  assert(fi == 24);

  if (!CheckStatus()) {
    return;
  }

  if (num_bits != 128) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(56);

  version = bits.ToUnsignedInt(56, 3);
  utc_hour = bits.ToUnsignedInt(59, 5);
  utc_min = bits.ToUnsignedInt(64, 6);
  position = bits.ToAisPoint(70, 49);
  pressure = bits.ToUnsignedInt(119, 9) + 799;  // hPa

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

std::ostream& operator<< (std::ostream &o, const Ais8_367_24 &msg) {
  return o << msg.mmsi << ": " << msg.version << ": " << msg.utc_hour << ": " << msg.utc_min << ": " << msg.pressure;
}

// SSW FI25 Satellite Ship Weather Tiny Version
Ais8_367_25::Ais8_367_25(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  assert(dac == 367);
  assert(fi == 25);

  if (!CheckStatus()) {
    return;
  }

  if (num_bits != 96) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bits.SeekTo(56);

  version = bits.ToUnsignedInt(56, 3);
  utc_hour = bits.ToUnsignedInt(59, 5);
  utc_min = bits.ToUnsignedInt(64, 6);
  pressure = bits.ToUnsignedInt(70, 9) + 799;  // hPa
  wind_speed = bits.ToUnsignedInt(79, 7);  // Knots
  wind_dir = bits.ToUnsignedInt(86, 9);  // Degrees

  spare = bits.ToUnsignedInt(95, 1);

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

std::ostream& operator<< (std::ostream &o, const Ais8_367_25 &msg) {
  return o << msg.mmsi << ": " << msg.version << ": " << msg.utc_hour << ": " << msg.utc_min << ": " << msg.pressure << ": " << msg.wind_speed << ": " << msg.wind_dir;
}

// Environmental Message
Ais8_367_33_Location::Ais8_367_33_Location(const AisBitset &bits,
                                           const size_t offset) {
  version = bits.ToUnsignedInt(offset, 6);
  position = bits.ToAisPoint(offset + 6, 55);

  precision = bits.ToUnsignedInt(offset + 61, 3);

  altitude_raw = bits.ToUnsignedInt(offset + 64, 12);
  altitude = altitude_raw / 10.;  // meters

  owner = bits.ToUnsignedInt(offset + 76, 4);
  timeout = bits.ToUnsignedInt(offset + 80, 3);
  spare2 = bits.ToUnsignedInt(offset + 83, 2);
}

Ais8_367_33_Station::Ais8_367_33_Station(const AisBitset &bits,
                                         const size_t offset) {
  name = bits.ToString(offset, 84);
  // Remove any trailing '@' characters.  Spec says they are not to be shown in the presentation.
  while (!name.empty() && name.back() == '@') {
    name.pop_back();
  }
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
    const size_t start = offset + idx * 26;
    currents[idx].speed_raw = bits.ToUnsignedInt(start, 8);
    currents[idx].speed = currents[idx].speed_raw / 10.;
    currents[idx].dir = bits.ToUnsignedInt(start + 8, 9);
    currents[idx].depth = bits.ToUnsignedInt(start + 17, 9);
  }
  type = bits.ToUnsignedInt(offset + 78, 3);
  spare2 = bits.ToUnsignedInt(offset + 81, 4);
}

Ais8_367_33_Curr3D::Ais8_367_33_Curr3D(const AisBitset &bits,
                                       const size_t offset) {
  for (size_t idx = 0; idx < 2; idx++) {
    const size_t start = offset + idx * 36;

    currents[idx].north_raw = bits.ToUnsignedInt(start, 9);
    currents[idx].north = currents[idx].north_raw / 10.;

    currents[idx].east_raw = bits.ToUnsignedInt(start + 9, 9);
    currents[idx].east = currents[idx].east_raw / 10.;

    currents[idx].up_raw = bits.ToUnsignedInt(start + 18, 9);
    currents[idx].up = currents[idx].up_raw / 10.;

    currents[idx].depth = bits.ToUnsignedInt(start + 27, 9);
  }
  type = bits.ToUnsignedInt(offset + 72, 3);
  spare2 = bits.ToUnsignedInt(offset + 75, 10);
}

Ais8_367_33_HorzFlow::Ais8_367_33_HorzFlow(const AisBitset &bits,
                                           const size_t offset) {
  bearing = bits.ToUnsignedInt(offset, 9);
  for (size_t idx = 0; idx < 2; idx++) {
    const size_t start = offset + 9 + (idx * 35);

    currents[idx].dist = bits.ToUnsignedInt(start, 9);

    currents[idx].speed_raw = bits.ToUnsignedInt(start + 9, 8);
    currents[idx].speed = currents[idx].speed_raw / 10.;

    currents[idx].dir = bits.ToUnsignedInt(start + 17, 9);
    currents[idx].level = bits.ToUnsignedInt(start + 26, 9);
  }
  type = bits.ToUnsignedInt(offset + 79, 3);
  spare2 = bits.ToUnsignedInt(offset + 82, 3);
}

Ais8_367_33_SeaState::Ais8_367_33_SeaState(const AisBitset &bits,
                                           const size_t offset) {
  swell_height_raw = bits.ToUnsignedInt(offset, 8);
  swell_height = swell_height_raw / 10.;

  swell_period = bits.ToUnsignedInt(offset + 8, 6);
  swell_dir = bits.ToUnsignedInt(offset + 14, 9);
  sea_state = bits.ToUnsignedInt(offset + 23, 4);
  swell_sensor_type = bits.ToUnsignedInt(offset + 27, 3);

  water_temp_raw = bits.ToInt(offset + 30, 10);
  water_temp = (water_temp_raw / 10.) - 10.0;

  water_temp_depth_raw = bits.ToUnsignedInt(offset + 40, 7);
  water_temp_depth = water_temp_depth_raw / 10.;

  water_sensor_type = bits.ToUnsignedInt(offset + 47, 3);

  wave_height_raw = bits.ToUnsignedInt(offset + 50, 8);
  wave_height = wave_height_raw / 10.;

  wave_period = bits.ToUnsignedInt(offset + 58, 6);
  wave_dir = bits.ToUnsignedInt(offset + 64, 9);
  wave_sensor_type = bits.ToUnsignedInt(offset + 73, 3);

  salinity_raw = bits.ToUnsignedInt(offset + 76, 9);
  salinity = salinity_raw / 10.;
}

Ais8_367_33_Salinity::Ais8_367_33_Salinity(const AisBitset &bits,
                                           const size_t offset) {
  water_temp_raw = bits.ToUnsignedInt(offset, 10);
  water_temp = water_temp_raw / 10. - 10;

  conductivity_raw = bits.ToUnsignedInt(offset + 10, 10);
  conductivity = conductivity_raw / 100.;

  pressure_raw = bits.ToUnsignedInt(offset + 20, 16);
  pressure = pressure_raw / 10.;

  salinity_raw = bits.ToUnsignedInt(offset + 36, 9);
  salinity = salinity_raw / 10.;

  salinity_type = bits.ToUnsignedInt(offset + 45, 2);
  sensor_type = bits.ToUnsignedInt(offset + 47, 3);
  spare2[0] = bits.ToUnsignedInt(offset + 50, 32);
  spare2[1] = bits.ToUnsignedInt(offset + 82, 3);
}

Ais8_367_33_Wx::Ais8_367_33_Wx(const AisBitset &bits,
                               const size_t offset) {
  air_temp_raw = bits.ToInt(offset, 11);
  air_temp = air_temp_raw / 10.;

  air_temp_sensor_type = bits.ToUnsignedInt(offset + 11, 3);
  precip = bits.ToUnsignedInt(offset + 14, 2);

  horz_vis_raw = bits.ToUnsignedInt(offset + 16, 8);
  horz_vis = horz_vis_raw / 10.;

  dew_point_raw = bits.ToUnsignedInt(offset + 24, 10);
  dew_point = (dew_point_raw / 10.) - 20.0;

  dew_point_type = bits.ToUnsignedInt(offset + 34, 3);

  air_pressure_raw = bits.ToUnsignedInt(offset + 37, 9);
  air_pressure = air_pressure_raw + 799;  // hPa.

  air_pressure_trend = bits.ToUnsignedInt(offset + 46, 2);
  air_pressure_sensor_type = bits.ToUnsignedInt(offset + 48, 3);

  salinity_raw = bits.ToUnsignedInt(offset + 51, 9);
  salinity = salinity_raw / 10.;

  spare2 = bits.ToUnsignedInt(offset + 60, 25);
}

Ais8_367_33_AirGap::Ais8_367_33_AirGap(const AisBitset &bits,
                                       const size_t offset) {
  air_draught = bits.ToUnsignedInt(offset, 13);
  air_gap = bits.ToUnsignedInt(offset + 13, 13);
  air_gap_trend = bits.ToUnsignedInt(offset + 26, 2);
  predicted_air_gap = bits.ToUnsignedInt(offset + 28, 13);
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
  wind_dir = bits.ToUnsignedInt(offset + 14, 9);
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

std::unique_ptr<Ais8_367_33_SensorReport>
ais8_367_33_sensor_report_factory(const AisBitset &bits,
                                  const size_t offset) {
  const auto rpt_type =
      (Ais8_367_33_SensorEnum)bits.ToUnsignedInt(offset, 4);

  // WARNING: out of order decoding
  // Only get the report header if we can decode the type

  const size_t rpt_start = offset + 27;  // Skip to after site_id
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
  case AIS8_367_33_SENSOR_ERROR: break;
  default:
    assert(false);
  }
  if (!rpt) {
    return {};
  }

  // Parse header
  rpt->report_type = rpt_type;
  bits.SeekTo(offset + 4);
  rpt->utc_day = bits.ToUnsignedInt(offset + 4, 5);
  rpt->utc_hr = bits.ToUnsignedInt(offset + 9, 5);
  rpt->utc_min = bits.ToUnsignedInt(offset + 14, 6);
  rpt->site_id = bits.ToUnsignedInt(offset + 20, 7);

  return std::unique_ptr<Ais8_367_33_SensorReport>(rpt);
}

Ais8_367_33::Ais8_367_33(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  assert(dac == 367);
  assert(fi == 33);

  if (!CheckStatus()) {
    return;
  }
  if (num_bits  < 168 || num_bits > 952) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  const size_t report_start = 56;
  const size_t num_sensor_reports = (num_bits - report_start) / AIS8_367_33_REPORT_SIZE;

  const size_t extra_bits = (num_bits - report_start) % AIS8_367_33_REPORT_SIZE;
  if (extra_bits > 0) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  for (size_t report_idx = 0; report_idx < num_sensor_reports; report_idx++) {
    const size_t start = report_start + (report_idx * AIS8_367_33_REPORT_SIZE);
    bits.SeekTo(start);
    std::unique_ptr<Ais8_367_33_SensorReport> sensor =
        ais8_367_33_sensor_report_factory(bits, start);
    if (sensor) {
      reports.push_back(std::move(sensor));
    } else {
      status = AIS_ERR_BAD_SUB_MSG;
      return;
    }
  }

  status = AIS_OK;
}

std::ostream& operator<< (std::ostream &o, const Ais8_367_33 &msg) {
  const int num_reports = msg.reports.size();
  for (int report_idx = 0; report_idx < num_reports; report_idx++) {
    Ais8_367_33_SensorEnum const report_type = msg.reports[report_idx]->report_type;
    switch(report_type) {
      case AIS8_367_33_SENSOR_LOCATION: {
        auto *rpt = dynamic_cast<Ais8_367_33_Location *>(msg.reports[report_idx].get());

        o << " [report_type: " << rpt->report_type << " day: " << rpt->utc_day << " hour: " << rpt->utc_hr << " min: " << rpt->utc_min << " site: " << rpt->site_id;
        o << " version: " << rpt->version << " position: " << rpt->position << " precision: " << rpt->precision << " altitude: " << rpt->altitude << " owner: " << rpt->owner << "]";
        break;
      }
      case AIS8_367_33_SENSOR_STATION: {
        auto *rpt = dynamic_cast<Ais8_367_33_Station *>(msg.reports[report_idx].get());

        o << " [report_type: " << rpt->report_type << " day: " << rpt->utc_day << " hour: " << rpt->utc_hr << " min: " << rpt->utc_min << " site: " << rpt->site_id;
        o << " name: " << rpt->name << "]";
        break;
      }
      case AIS8_367_33_SENSOR_WIND: {
        auto *rpt = dynamic_cast<Ais8_367_33_Wind *>(msg.reports[report_idx].get());

        o << " [report_type: " << rpt->report_type << " day: " << rpt->utc_day << " hour: " << rpt->utc_hr << " min: " << rpt->utc_min << " site: " << rpt->site_id;
        o << " speed: " << rpt->wind_speed << " gust: " << rpt->wind_gust << " dir: " << rpt->wind_dir;
        o << " wind_gust_dir: " << rpt->wind_gust_dir << " sensor_type: " << rpt->sensor_type;
        o << " wind_forecast: " << rpt->wind_forecast << " wind_gust_forecast: " << rpt->wind_gust_forecast << " wind_dir_forecast: " << rpt->wind_dir_forecast;
        o << " utc_day_forecast: " << rpt->utc_day_forecast << " utc_hour_forecast: " << rpt->utc_hour_forecast << " utc_min_forecast: " << rpt->utc_min_forecast << " duration: " << rpt->duration << "]";
        break;
      }
      case AIS8_367_33_SENSOR_WATER_LEVEL: {
        auto *rpt = dynamic_cast<Ais8_367_33_WaterLevel *>(msg.reports[report_idx].get());

        o << " [report_type: " << rpt->report_type << " day: " << rpt->utc_day << " hour: " << rpt->utc_hr << " min: " << rpt->utc_min << " site: " << rpt->site_id;
        o << " type: " << rpt->type << " level: " << rpt->level << " trend: " << rpt->trend << " vdatum: " << rpt->vdatum;
        o << " sensor_type: " << rpt->sensor_type << " forecast_type: " << rpt->forecast_type;
        o << " level_forecast: " << rpt->level_forecast;
        o << " utc_day_forecast: " << rpt->utc_day_forecast << " utc_hour_forecast: " << rpt->utc_hour_forecast << " utc_min_forecast: " << rpt->utc_min_forecast << " duration: " << rpt->duration << "]";
        break;
      }
      case AIS8_367_33_SENSOR_CURR_2D: {
        auto *rpt = dynamic_cast<Ais8_367_33_Curr2D *>(msg.reports[report_idx].get());

        o << " [report_type: " << rpt->report_type << " day: " << rpt->utc_day << " hour: " << rpt->utc_hr << " min: " << rpt->utc_min << " site: " << rpt->site_id;
        for (size_t idx = 0; idx < 3; idx++) {
          o << " [speed: " << rpt->currents[idx].speed << " dir: " << rpt->currents[idx].dir << " depth: " << rpt->currents[idx].depth << "]";
        }
        o << " type: " << rpt->type << "]";
        break;
      }
      case AIS8_367_33_SENSOR_CURR_3D: {
        auto *rpt = dynamic_cast<Ais8_367_33_Curr3D *>(msg.reports[report_idx].get());

        o << " [report_type: " << rpt->report_type << " day: " << rpt->utc_day << " hour: " << rpt->utc_hr << " min: " << rpt->utc_min << " site: " << rpt->site_id;
        for (size_t idx = 0; idx < 2; idx++) {
          o << " [north: " << rpt->currents[idx].north << " east: " << rpt->currents[idx].east << " up: " << rpt->currents[idx].up << " depth: " << rpt->currents[idx].depth << "]";
        }
        o << " type: " << rpt->type << "]";
        break;
      }
      case AIS8_367_33_SENSOR_HORZ_FLOW: {
        auto *rpt = dynamic_cast<Ais8_367_33_HorzFlow *>(msg.reports[report_idx].get());

        o << " [report_type: " << rpt->report_type << " day: " << rpt->utc_day << " hour: " << rpt->utc_hr << " min: " << rpt->utc_min << " site: " << rpt->site_id;
        o << " bearing: " << rpt->bearing;
        for (size_t idx = 0; idx < 2; idx++) {
          o << " [dist: " << rpt->currents[idx].dist << " speed: " << rpt->currents[idx].speed << " dir: " << rpt->currents[idx].dir << " level: " << rpt->currents[idx].level << "]";
        o << " type: " << rpt->type << "]";
        }
        break;
      }
      case AIS8_367_33_SENSOR_SEA_STATE: {
        auto *rpt = dynamic_cast<Ais8_367_33_SeaState *>(msg.reports[report_idx].get());

        o << " [report_type: " << rpt->report_type << " day: " << rpt->utc_day << " hour: " << rpt->utc_hr << " min: " << rpt->utc_min << " site: " << rpt->site_id;
        o << " swell_height: " << rpt->swell_height << " swell_period: " << rpt->swell_period << " swell_dir: " << rpt->swell_dir;
        o << " sea_state: " << rpt->sea_state << " sensor_type: " << rpt->swell_sensor_type;
        o << " water_temp: " << rpt->water_temp << " water_temp_depth: " << rpt->water_temp_depth << " water_sensor_type: " << rpt->water_sensor_type;
        o << " wave_height: " << rpt->wave_height << " wave_period: " << rpt->wave_period << " wave_dir: " << rpt->wave_dir << " wave_sensor_type: " << rpt->wave_sensor_type;
        o << " salinity: " << rpt->salinity << "]";
        break;
      }
      case AIS8_367_33_SENSOR_SALINITY: {
        auto *rpt = dynamic_cast<Ais8_367_33_Salinity *>(msg.reports[report_idx].get());

        o << " [report_type: " << rpt->report_type << " day: " << rpt->utc_day << " hour: " << rpt->utc_hr << " min: " << rpt->utc_min << " site: " << rpt->site_id;
        o << " water_temp: " << rpt->water_temp << " conductivity: " << rpt->conductivity << " water_pressure: " << rpt->pressure;
        o << " salinity: " << rpt->salinity << " salinity_type: " << rpt->salinity_type << " sensor_type: " << rpt->sensor_type << "]";
        break;
      }
      case AIS8_367_33_SENSOR_WX: {
        auto *rpt = dynamic_cast<Ais8_367_33_Wx *>(msg.reports[report_idx].get());

        o << " [report_type: " << rpt->report_type << " day: " << rpt->utc_day << " hour: " << rpt->utc_hr << " min: " << rpt->utc_min << " site: " << rpt->site_id;
        o << " air_temp: " << rpt->air_temp << " air_temp_sensor_type: " << rpt->air_temp_sensor_type << " precip: " << rpt->precip;
        o << " horz_vis: " << rpt->horz_vis << " dew_point: " << rpt->dew_point << " dew_point_type: " << rpt->dew_point_type;
        o << " air_pressure: " << rpt->air_pressure << " air_pressure_trend: " << rpt->air_pressure_trend << " air_pressure_sensor_type: " << rpt->air_pressure_sensor_type;
        o << " salinity: " << rpt->salinity << "]";
        break;
      }
      case AIS8_367_33_SENSOR_AIR_GAP: {
        auto *rpt = dynamic_cast<Ais8_367_33_AirGap *>(msg.reports[report_idx].get());

        o << " [report_type: " << rpt->report_type << " day: " << rpt->utc_day << " hour: " << rpt->utc_hr << " min: " << rpt->utc_min << " site: " << rpt->site_id;
        o << " air_draught: " << rpt->air_draught << " air_gap: " << rpt->air_gap << " air_gap_trend: " << rpt->air_gap_trend;
        o << " predicted_air_gap: " << rpt->predicted_air_gap;
        o << " utc_day_forecast: " << rpt->utc_day_forecast << " utc_hour_forecast: " << rpt->utc_hour_forecast << " utc_min_forecast: " << rpt->utc_min_forecast << " type: " << rpt->type << "]";
        break;
      }
      case AIS8_367_33_SENSOR_WIND_REPORT_2: {
        auto *rpt = dynamic_cast<Ais8_367_33_Wind_V2 *>(msg.reports[report_idx].get());

        o << " [report_type: " << rpt->report_type << " day: " << rpt->utc_day << " hour: " << rpt->utc_hr << " min: " << rpt->utc_min << " site: " << rpt->site_id;
        o << " wind_speed: " << rpt->wind_speed << " wind_gust: " << rpt->wind_gust << " wind_dir: " << rpt->wind_dir;
        o << " averaging_time: " << rpt->averaging_time << " sensor_type: " << rpt->sensor_type;
        o << " wind_speed_forecast: " << rpt->wind_speed_forecast << " wind_gust_forecast: " << rpt->wind_gust_forecast;
        o << " wind_dir_forecast: " << rpt->wind_dir_forecast;

        o << " utc_hour_forecast: " << rpt->utc_hour_forecast << " utc_min_forecast: " << rpt->utc_min_forecast << " duration: " << rpt->duration << "]";
        break;
      }
      default: break;
    } // switch
  } // for

  return o;
}

}  // namespace libais
