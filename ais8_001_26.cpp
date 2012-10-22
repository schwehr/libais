// The USCG RTCM SC 121 Environmental Sensor Message tested in the Tampa, FL from 200[78] util ?

// http://www.dtic.mil/cgi-bin/GetTRDoc?AD=ADA504755
// Phase I Summary Report on AIS Transmit Project (Environmental Message)
// TODO: should print methods be to stdout?

#include "ais.h"


Ais8_1_26_Location::Ais8_1_26_Location(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
  x = sbits(bs, offset, 28) / 600000.;
  y = sbits(bs, offset+28, 27) / 600000.;
  z = ubits(bs, offset+55, 11) / 10.;
  owner = ubits(bs, offset+66, 4);
  timeout = ubits(bs, offset+70, 3);
  spare = ubits(bs, offset+73, 12);
}

void
Ais8_1_26_Location::print() {
  std::cerr << "Loca: " << x << " " << y << " " << z
            << " " << owner << " " << timeout
            << "\n";
}


Ais8_1_26_Station::Ais8_1_26_Station(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
  name = ais_str(bs, offset, 84);
  spare = ubits(bs, offset+84, 1);
}

void Ais8_1_26_Station::print() {
  std::cerr << "Stat: " << name << "\n";
}

Ais8_1_26_Wind::Ais8_1_26_Wind(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
  wind_speed = ubits(bs, offset, 7);
  wind_gust  = ubits(bs, offset+7, 7); // knots
  wind_dir = ubits(bs, offset+14, 9);
  wind_gust_dir = ubits(bs, offset+23, 9);
  sensor_type = ubits(bs, offset+32, 3);
  wind_forcast = ubits(bs, offset+35, 7);
  wind_gust_forcast = ubits(bs, offset+42, 7); // knots
  wind_dir_forcast = ubits(bs, offset+49, 9);
  utc_day_forcast = ubits(bs, offset+58, 5);
  utc_hour_forcast = ubits(bs, offset+63, 5);
  utc_min_forcast = ubits(bs, offset+68, 6);
  duration = ubits(bs, offset+74, 8);
  spare = ubits(bs, offset+82, 3);
}
void Ais8_1_26_Wind::print () {
  std::cerr << "Wind: " << wind_speed << " " << wind_dir << " " << sensor_type << " " << duration << "\n";
}

Ais8_1_26_WaterLevel::Ais8_1_26_WaterLevel(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
  type = bs[offset];
  level = sbits(bs, offset+1, 16) / 100.;
  trend = ubits(bs, offset+17, 2);
  vdatum = ubits(bs, offset+19, 5);
  sensor_type = ubits(bs, offset+24, 3);
  forcast_type = bs[offset+27];
  level_forcast = sbits(bs, offset+28, 16) / 100.;
  utc_day_forcast = ubits(bs, offset+44, 5);
  utc_hour_forcast = ubits(bs, offset+49, 5);
  utc_min_forcast = ubits(bs, offset+54, 6);
  duration = ubits(bs, offset+60, 8);
  spare = ubits(bs, offset+68, 17);
}

void Ais8_1_26_WaterLevel::print() {
    std::cerr << "WLvl" << level << " " << vdatum << " " << level_forcast << "\n";
}


Ais8_1_26_Curr2D::Ais8_1_26_Curr2D(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
  for (size_t idx=0; idx < 3; idx++) {
    size_t start = offset + idx * 26;
    currents[idx].speed = ubits(bs, start, 8) / 10.;
    currents[idx].dir = ubits(bs, start+8, 9);
    currents[idx].depth = ubits(bs, start+17, 9);
  }
  type = ubits(bs, offset+78, 3);
  spare = ubits(bs, offset+81, 4);
}

void Ais8_1_26_Curr2D::print() {
  std::cerr << "Cr2D: " << currents[0].speed << " " << currents[1].speed << " "
            << currents[2].speed << " " << type << "\n";
}


Ais8_1_26_Curr3D::Ais8_1_26_Curr3D(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
  for (size_t idx=0; idx < 2; idx++) {
    size_t start = offset + idx * 33;
    currents[idx].north = ubits (bs, start, 8) / 10.;
    currents[idx].east = ubits (bs, start+8, 8) / 10.;
    currents[idx].up = ubits (bs, start+16, 8) / 10.;
    currents[idx].depth = ubits (bs, start+24, 9);
  }
  type = ubits (bs, offset+66, 3);
  spare = ubits (bs, offset+69, 16);
}

void Ais8_1_26_Curr3D::print() {
  std::cerr << "Cr3D: "
            << currents[0].north << " " << currents[0].east << " " << currents[0].up << " " << currents[0].depth << " "
            << currents[1].north << " " << currents[1].east << " " << currents[1].up << " " << currents[1].depth
            << "\n";
}


Ais8_1_26_HorzFlow::Ais8_1_26_HorzFlow(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
  for (size_t idx=0; idx < 2; idx++) {
    size_t start = offset + idx * 42;
    currents[idx].bearing = ubits(bs, start, 9);
    currents[idx].dist = ubits(bs, start+9, 7);
    currents[idx].speed = ubits(bs, start+16, 8) / 10.;
    currents[idx].dir = ubits(bs, start+24, 9);
    currents[idx].level = ubits(bs, start+33, 9);
  }
  spare = bs[offset+84];
}

void Ais8_1_26_HorzFlow::print() {
  std::cerr << "CrHz: "
            << currents[0].speed << " " << currents[0].dir << " " << currents[0].level << " "
            << currents[1].speed << " " << currents[1].dir << " " << currents[1].level << " "
            << "\n";
}

Ais8_1_26_SeaState::Ais8_1_26_SeaState(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
  swell_height = ubits(bs, offset, 8) / 10.;
  swell_period = ubits(bs, offset+8, 6);
  swell_dir = ubits(bs, offset+14, 9) ;
  sea_state = ubits(bs, offset+23, 4);
  swell_sensor_type = ubits(bs, offset+27, 3);
  water_temp = sbits(bs, offset+30, 10) / 10.;
  water_temp_depth = ubits(bs, offset+40, 7) / 10.;
  water_sensor_type = ubits(bs, offset+47, 3);
  wave_height = ubits(bs, offset+50, 8) / 10.;
  wave_period = ubits(bs, offset+58, 6);
  wave_dir = ubits(bs, offset+64, 9);
  wave_sensor_type = ubits(bs, offset+73, 3);
  salinity = ubits(bs, offset+76, 9) / 10.;
}

void Ais8_1_26_SeaState::print() {
    std::cerr << "Stat: " << swell_height << " " << sea_state<< " " << water_temp << " " << wave_height << "\n";
}

Ais8_1_26_Salinity::Ais8_1_26_Salinity(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
  water_temp = ubits(bs, offset, 10) / 10. - 10;
  conductivity = ubits(bs, offset+10, 10) / 100.;
  pressure = ubits(bs, offset+20, 16) / 10.;
  salinity = ubits(bs, offset+36, 9) / 10.;
  salinity_type = ubits(bs, offset+45, 2);
  sensor_type = ubits(bs, offset+47, 3);
  spare[0] = ubits(bs, offset+50, 32);
  spare[1] = ubits(bs, offset+82, 3);
}

void Ais8_1_26_Salinity::print() {
    std::cerr << "Salt: " << water_temp << " " << conductivity << " " << pressure << " " << salinity << "\n";
}

Ais8_1_26_Wx::Ais8_1_26_Wx(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
  air_temp = sbits(bs, offset, 11) / 10.;
  air_temp_sensor_type = ubits(bs, offset+11, 3);
  precip = ubits(bs, offset+14, 2);
  horz_vis = ubits(bs, offset+16, 8) / 10.;
  dew_point = sbits(bs, offset+24, 10)/ 10.;
  dew_point_type = ubits(bs, offset+34, 3);
  air_pressure = ubits(bs, offset+37, 9);
  air_pressure_trend = ubits(bs, offset+46, 2);
  air_pressor_type = ubits(bs, offset+48, 3);
  salinity = ubits(bs, offset+51, 9)/ 10.;
  spare = ubits(bs, offset+60, 25);
}

void Ais8_1_26_Wx::print() {
    std::cerr << "Wx:   " << air_temp << " " << precip << " " << dew_point <<  " " <<  air_pressure << " " << salinity << "\n";
}

Ais8_1_26_AirDraught::Ais8_1_26_AirDraught(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
  draught = ubits(bs, offset, 13) / 100.;
  gap = ubits(bs, offset+13, 13) / 10.;
  trend = ubits(bs, offset+26, 2);
  forcast_gap = ubits(bs, offset+28, 13) / 10.;
  utc_day_forcast = ubits(bs, offset+41, 5);
  utc_hour_forcast = ubits(bs, offset+46, 5);
  utc_min_forcast = ubits(bs, offset+51, 6);
  spare = ubits(bs, offset+57, 28);
}

void Ais8_1_26_AirDraught::print () {
    std::cerr << "ADft: " << draught << " " << gap << " " << " " << forcast_gap << "\n";
}

Ais8_1_26_SensorReport* ais8_1_26_sensor_report_factory(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
  const Ais8_1_26_SensorEnum rpt_type = (Ais8_1_26_SensorEnum)ubits(bs, offset, 4);
  const size_t rpt_start = offset+27;  // skip tp after site_id

  Ais8_1_26_SensorReport *rpt = 0;
  switch(rpt_type) {
  case AIS8_1_26_SENSOR_LOCATION:    rpt = new Ais8_1_26_Location(bs,rpt_start); break;
  case AIS8_1_26_SENSOR_STATION:     rpt = new Ais8_1_26_Station(bs, rpt_start); break;
  case AIS8_1_26_SENSOR_WIND:        rpt = new Ais8_1_26_Wind(bs, rpt_start); break;
  case AIS8_1_26_SENSOR_WATER_LEVEL: rpt = new Ais8_1_26_WaterLevel(bs, rpt_start); break;
  case AIS8_1_26_SENSOR_CURR_2D:     rpt = new Ais8_1_26_Curr2D(bs, rpt_start); break;
  case AIS8_1_26_SENSOR_CURR_3D:     rpt = new Ais8_1_26_Curr3D(bs, rpt_start); break;
  case AIS8_1_26_SENSOR_HORZ_FLOW:   rpt = new Ais8_1_26_HorzFlow(bs, rpt_start); break;
  case AIS8_1_26_SENSOR_SEA_STATE:   rpt = new Ais8_1_26_SeaState(bs, rpt_start); break;
  case AIS8_1_26_SENSOR_SALINITY:    rpt = new Ais8_1_26_Salinity(bs, rpt_start); break;
  case AIS8_1_26_SENSOR_WX:          rpt = new Ais8_1_26_Wx(bs, rpt_start); break;
  case AIS8_1_26_SENSOR_AIR_DRAUGHT: rpt = new Ais8_1_26_AirDraught(bs, rpt_start); break;
  case AIS8_1_26_SENSOR_RESERVED_11: std::cerr << "NOT handled " << rpt_type << "\n";   break;
  case AIS8_1_26_SENSOR_RESERVED_12: std::cerr << "NOT handled " << rpt_type << "\n";   break;
  case AIS8_1_26_SENSOR_RESERVED_13: std::cerr << "NOT handled " << rpt_type << "\n";   break;
  case AIS8_1_26_SENSOR_RESERVED_14: std::cerr << "NOT handled " << rpt_type << "\n";   break;
  case AIS8_1_26_SENSOR_RESERVED_15: std::cerr << "NOT handled " << rpt_type << "\n";   break;
  default:
    std::cerr << "ERROR: unhandle sensor report type: " << rpt_type << "\n";
  }
  if (rpt) {
    rpt->report_type = rpt_type;
    rpt->utc_day = ubits(bs, offset+4, 5);
    rpt->utc_hr = ubits(bs, offset+9, 5);
    rpt->utc_min = ubits(bs, offset+14, 6);
    rpt->site_id = ubits(bs, offset+20, 7);
  }

  return rpt;
}


Ais8_1_26::Ais8_1_26(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  assert(pad<6);
  init();
  const int num_bits = strlen(nmea_payload) * 6 - pad;
  if (168 > num_bits || num_bits > 1098) { status = AIS_ERR_BAD_BIT_COUNT; return; }

  std::bitset<MAX_BITS> bs;
  status = aivdm_to_bits(bs, nmea_payload);
  if (had_error()) return;

  if (!decode_header8(bs)) return;
  if (1 != dac or 26 != fi) { status = AIS_ERR_WRONG_MSG_TYPE; return; }

  const int num_sensor_reports = (num_bits - 56) / AIS8_1_26_REPORT_SIZE;
  if ((num_bits - 56) % AIS8_1_26_REPORT_SIZE) {
    std::cerr << "WARNING: extra data in sensor report msg 8_1_26\n";
  }

  for(size_t report_idx=0; report_idx < num_sensor_reports; report_idx++) {
    const size_t start = 56 + report_idx * AIS8_1_26_REPORT_SIZE;
    Ais8_1_26_SensorReport *sensor_report = ais8_1_26_sensor_report_factory(bs, start);
    if (sensor_report) {
        reports.push_back(sensor_report);
    } else std::cerr << "Failed to decode sensor report\n";

  }
}

void Ais8_1_26::print() {
  std::cout << "BBM_imo_1_8_26_Environmental: " << message_id
            << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
  for(size_t rpt_idx=0; rpt_idx < reports.size(); rpt_idx++) {
      std::cerr << "\t" << rpt_idx << ": ";
      reports[rpt_idx]->print();
  }
}


Ais8_1_26::~Ais8_1_26() {
  for (size_t i=0; i < reports.size(); i++) delete reports[i];
}
