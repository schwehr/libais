// -*- c++ -*-

// TODO(schwehr): should all messages just use MAX_BITS or should it
//                be set for each message?
// TODO(schwehr): create an archive of messages to do not decode.  Can
//                libais be made to safely decode any of them?

#ifndef AIS_H
#define AIS_H

#include <bitset>
#include <cassert>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>

using std::bitset;
using std::ostream;
using std::string;
using std::vector;

#define LIBAIS_VERSION_MAJOR 0
#define LIBAIS_VERSION_MINOR 13

// Returns vector of the text between the delimiters.  Uses an empty string
// for empty fields.  Empty string returns a vector of length 1 containing an
// empty string.
// Empty delim_str is not allowed.
vector<string> Split(const string &str, const string &delim_str);

// Returns the text in the nth field starting with the first field being 0.
// Empty delim_str is not allowed.
string GetNthField(const string &str, const size_t n, const string &delim_str);

// Returns the number of pad bits in an AIS AIVDM NMEA string.
// Returns -1 if there is an error.
int GetPad(const string &nmea_str);

// Returns the armored payload of an AIS AIVDM NMEA string.
// Returns an empty string if there was an error.
string GetBody(const string &nmea_str);

extern bool nmea_ord_initialized;  // If false, call build_nmea_lookup
bitset<6> Reverse(const bitset<6> &bits);
void BuildNmeaLookup();

static const int MAX_BITS = 1192;

enum AIS_STATUS {
  AIS_UNINITIALIZED,  // Message is not yet completely decoded.
  AIS_OK,
  AIS_ERR_BAD_BIT_COUNT,
  AIS_ERR_BAD_NMEA_CHR,
  AIS_ERR_BAD_PTR,
  AIS_ERR_UNKNOWN_MSG_TYPE,
  AIS_ERR_MSG_NOT_IMPLEMENTED,
  AIS_ERR_MSG_SUB_NOT_IMPLEMENTED,
  AIS_ERR_EXPECTED_STRING,
  AIS_ERR_BAD_MSG_CONTENT,
  AIS_ERR_MSG_TOO_LONG,
  AIS_ERR_BAD_SUB_MSG,
  AIS_ERR_BAD_SUB_SUB_MSG,
  AIS_STATUS_NUM_CODES
};

extern const char *const AIS_STATUS_STRINGS[AIS_STATUS_NUM_CODES];

struct AisPoint {
  float x, y;  // TODO(schwehr): change all x, y to lng_deg, lat_deg.
};

class AisMsg {
 public:
  int message_id;
  int repeat_indicator;
  int mmsi;

  // TODO(schwehr): make status private and have accessors.
  bool had_error() const {  return status != AIS_OK;  }
  AIS_STATUS get_error() const {return status;}

 protected:
  AIS_STATUS status;  // AIS_OK or error code

  AisMsg() : status(AIS_UNINITIALIZED) {}
  AisMsg(const char *nmea_payload, const size_t pad);
};

// TODO(schwehr): factor out commstate from all messages?
class Ais1_2_3 : public AisMsg {
 public:
  int nav_status;
  bool rot_over_range;
  int rot_raw;
  float rot;
  float sog;
  int position_accuracy;
  float x, y;
  float cog;
  int true_heading;
  int timestamp;
  int special_manoeuvre;
  int spare;
  bool raim;

  // COMM state SOTDMA msgs 1 and 2
  int sync_state;  // SOTDMA and ITDMA
  int slot_timeout;
  bool slot_timeout_valid;

  // Based on slot_timeout which ones are valid
  int received_stations;
  bool received_stations_valid;

  int slot_number;
  bool slot_number_valid;

  bool utc_valid;
  int utc_hour;
  int utc_min;
  int utc_spare;

  int slot_offset;
  bool slot_offset_valid;

  // ITDMA - msg type 3
  int slot_increment;
  bool slot_increment_valid;

  int slots_to_allocate;
  bool slots_to_allocate_valid;

  bool keep_flag;  // 3.3.7.3.2 Annex 2 ITDMA.  Table 20
  bool keep_flag_valid;

  Ais1_2_3(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais1_2_3 &msg);

// 4 bsreport and 11 utc date response
class Ais4_11 : public AisMsg {
 public:
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  int position_accuracy;
  float x, y;
  int fix_type;
  int transmission_ctl;
  int spare;
  bool raim;

  // COMM state SOTDMA msgs 1 and 2
  int sync_state;
  int slot_timeout;

  // Based on slot_timeout which ones are valid
  int received_stations;
  bool received_stations_valid;

  int slot_number;
  bool slot_number_valid;

  bool utc_valid;
  int utc_hour;
  int utc_min;
  int utc_spare;

  int slot_offset;
  bool slot_offset_valid;

  // **NO** ITDMA
  Ais4_11(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais4_11 &msg);

class Ais5 : public AisMsg {
 public:
  int ais_version;
  int imo_num;
  string callsign;
  string name;
  int type_and_cargo;
  int dim_a;
  int dim_b;
  int dim_c;
  int dim_d;
  int fix_type;
  int eta_month;
  int eta_day;
  int eta_hour;
  int eta_minute;
  float draught;  // present static draft. m
  string destination;
  int dte;
  int spare;

  Ais5(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais5 &msg);

// Addessed binary message (ABM)
const size_t AIS6_MAX_BITS = MAX_BITS;

// AIS Binary Broadcast message ... parent to many
class Ais6 : public AisMsg {
 public:
  static const int MAX_BITS = AIS6_MAX_BITS;

  int seq;  // sequence number
  int mmsi_dest;
  bool retransmit;
  int spare;
  int dac;  // dac+fi = app id
  int fi;

  // TODO(schwehr): how to make Ais6 protected?
  Ais6(const char *nmea_payload, const size_t pad);

 protected:
  Ais6() {}
};
ostream& operator<< (ostream &o, const Ais6 &msg);

// Text message.  ITU 1371-1
class Ais6_1_0 : public Ais6 {
 public:
  bool ack_required;
  int msg_seq;
  string text;
  int spare2;

  Ais6_1_0(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais6_1_0 &msg);

// Application ack.  ITU 1371-1
class Ais6_1_1 : public Ais6 {
 public:
  int ack_dac;
  int msg_seq;
  int spare2;

  Ais6_1_1(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais6_1_1 &msg);

// Interrogation for a DAC/FI.  ITU 1371-1
class Ais6_1_2 : public Ais6 {
 public:
  int req_dac;
  int req_fi;
  // TODO(schwehr): spare2?

  Ais6_1_2(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais6_1_2 &msg);

// Capability interogation.  ITU 1371-1
class Ais6_1_3 : public Ais6 {
 public:
  int req_dac;
  int spare2;

  Ais6_1_3(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais6_1_3 &msg);

// Capability interogation reply.  ITU 1371-1
class Ais6_1_4 : public Ais6 {
 public:
  int ack_dac;
  int capabilities[64];
  int cap_reserved[64];
  int spare2;

  Ais6_1_4(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais6_1_4 &msg);

// Number of persons on board.  ITU 1371-1
class Ais6_1_40 : public Ais6 {
 public:
  int persons;
  int spare2;

  Ais6_1_40(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais6_1_40 &msg);

// IMO Circ 236 Dangerous cargo indication
// Not to be transmitted after 2012-Jan-01
class Ais6_1_12 : public Ais6 {
 public:
  string last_port;
  int utc_month_dep;  // actual time of departure
  int utc_day_dep, utc_hour_dep, utc_min_dep;
  string next_port;
  int utc_month_next;  // estimated arrival
  int utc_day_next, utc_hour_next, utc_min_next;
  string main_danger;
  string imo_cat;
  int un;
  int value;  // TODO(schwehr): units?
  int value_unit;
  int spare2;

  Ais6_1_12(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais6_1_12 &msg);


struct Ais6_1_14_Window {
  float y, x;
  int utc_hour_from, utc_min_from;
  int utc_hour_to, utc_min_to;
  int cur_dir;
  float cur_speed;
};


// IMO Circ 236 Tidal window
// Not to be transmitted after 2012-Jan-01
class Ais6_1_14 : public Ais6 {
 public:
  int utc_month, utc_day;
  vector<Ais6_1_14_Window> windows;

  Ais6_1_14(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais6_1_14 &msg);


// IMO Circ 289 Clearance time to enter port
class Ais6_1_18 : public Ais6 {
 public:
  int link_id;
  int utc_month, utc_day, utc_hour, utc_min;
  string port_berth, dest;
  float x, y;
  int spare2[2];  // 32 bits per spare

  Ais6_1_18(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais6_1_18 &msg);


// IMO Circ 289 Berthing data
class Ais6_1_20 : public Ais6 {
 public:
  int link_id;
  int length;
  float depth;
  int position;
  int utc_month, utc_day, utc_hour, utc_min;
  bool services_known;
  // TODO(schwehr): enum of service types
  int services[26];
  string name;
  float x, y;

  Ais6_1_20(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais6_1_20 &msg);

// TODO(schwehr): Make cargo a class and set all to invalid in constructor
struct Ais6_1_25_Cargo {
  int code_type;
  bool imdg_valid;  // also set with BC
  int imdg;
  bool spare_valid;
  int spare;  // imdg or dc or marpols
  bool un_valid;
  int un;
  bool bc_valid;
  int bc;
  bool marpol_oil_valid;
  int marpol_oil;
  bool marpol_cat_valid;
  int marpol_cat;
};

// IMO Circ 289 Dangerous cargo indication 2
// Replaces 8_1_12?
class Ais6_1_25 : public Ais6 {
 public:
  int amount_unit;
  int amount;

  vector<Ais6_1_25_Cargo> cargos;  // 0 to 17 cargo entries

  Ais6_1_25(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais6_1_25 &msg);


// TODO(schwehr): addressed sensor report 6_1_26

// IMO Circ 289 Route information
class Ais6_1_28 : public Ais6 {
 public:
  int link_id;
  int sender_type, route_type;
  int utc_month_start, utc_day_start, utc_hour_start, utc_min_start;
  int duration;
  vector<AisPoint> waypoints;

  Ais6_1_28(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais6_1_28 &msg);


// IMO Circ 289 Text description
class Ais6_1_30 : public Ais6 {
 public:
  int link_id;
  string text;

  Ais6_1_30(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais6_1_30 &msg);


// IMO Circ 289
// Warning: The bit encoding for 6_1_14_Window and 6_1_32 on
//   the wire has x and y in a different order.
// TODO(schwehr): Reuse Ais6_1_14_Window
struct Ais6_1_32_Window {
  float x, y;
  int from_utc_hour, from_utc_min;
  int to_utc_hour, to_utc_min;
  int cur_dir;
  float cur_speed;  // knots
};


// IMO Circ 289 Tidal window
class Ais6_1_32 : public Ais6 {
 public:
  int utc_month;
  int utc_day;
  vector<Ais6_1_32_Window> windows;

  Ais6_1_32(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais6_1_32 &msg);


//////////////////////////////////////////////////////////////////////

// 7 and 13 are ACKs for msg 6 and 12
class Ais7_13 : public AisMsg {
 public:
  int spare;

  vector<int> dest_mmsi;
  vector<int> seq_num;

  Ais7_13(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais7_13 &msg);

const size_t AIS8_MAX_BITS = 1192;

// AIS Binary Broadcast message ... parent to many
class Ais8 : public AisMsg {
 public:
  int spare;
  // TODO(schwehr): seq? // ITU M.R. 1371-3 Anex 2 5.3.1
  int dac;  // dac+fi = app id
  int fi;

  // TODO(schwehr): make Ais8 protected
  Ais8(const char *nmea_payload, const size_t pad);
 protected:
  Ais8() {}
};
ostream& operator<< (ostream &o, const Ais8 &msg);

// Text telegram ITU 1371-1
class Ais8_1_0 : public Ais8 {
 public:
  bool ack_required;
  int msg_seq;
  string text;
  int spare2;

  Ais8_1_0(const char *nmea_payload, size_t pad);
};
ostream& operator<< (ostream &o, const Ais8_1_0 &msg);

// 8_1_1 No message
// 8_1_2 No message
// 8_1_3 No message
// 8_1_4 No message

// Persons on board ITU 1371-1
class Ais8_1_40 : public Ais8 {
 public:
  int persons;
  int spare2;
  Ais8_1_40(const char *nmea_payload, size_t pad);
};
ostream& operator<< (ostream &o, const Ais8_1_40 &msg);

// IMO Circ 289 met hydro - Not to be transmitted after 2013-Jan-01
// See also IMO Circ 236
class Ais8_1_11 : public Ais8 {
 public:
  float y, x;
  int day;
  int hour;
  int minute;
  int wind_ave;  // kts
  int wind_gust;  // kts
  int wind_dir;
  int wind_gust_dir;
  float air_temp;  // C
  int rel_humid;
  float dew_point;
  float air_pres;
  int air_pres_trend;
  float horz_vis;  // NM
  float water_level;  // m
  int water_level_trend;
  float surf_cur_speed;
  int surf_cur_dir;
  float cur_speed_2;  // kts
  int cur_dir_2;
  int cur_depth_2;  // m
  float cur_speed_3;  // kts
  int cur_dir_3;
  int cur_depth_3;  // m
  float wave_height;  // m
  int wave_period;
  int wave_dir;
  float swell_height;
  int swell_period;
  int swell_dir;
  int sea_state;  // beaufort scale
  float water_temp;
  int precip_type;
  float salinity;
  int ice;  // yes/no/undef/unknown
  int spare2;
  int extended_water_level;  // OHMEX uses this for extra water level precision

  Ais8_1_11(const char *nmea_payload, size_t pad);
};
ostream& operator<< (ostream &o, const Ais8_1_11 &msg);


// IMO Circ 236 Fairway closed - Not to be transmitted after 2012-Jan-01
class Ais8_1_13 : public Ais8 {
 public:
  string reason, location_from, location_to;
  int radius;
  int units;
  // TODO(schwehr): utc?  warning: day/month out of order
  int day_from, month_from, hour_from, minute_from;
  int day_to, month_to, hour_to, minute_to;
  int spare2;

  Ais8_1_13(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais8_1_13 &msg);

// IMO Circ 236 Extended ship static and voyage data
// Not to be transmitted after 2012-Jan-01
class Ais8_1_15 : public Ais8 {
 public:
  float air_draught;
  int spare2;

  Ais8_1_15(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais8_1_15 &msg);

// IMO Circ 236 Number of persons on board
class Ais8_1_16 : public Ais8 {
 public:
  int persons;
  int spare2;

  Ais8_1_16(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais8_1_16 &msg);

struct Ais8_1_17_Target {
  int type;
  string id;
  int spare;
  float y, x;
  int cog;
  int timestamp;
  int sog;
};

// IMO Circ 236 VTS Generated/synthetic targets
class Ais8_1_17 : public Ais8 {
 public:
  vector<Ais8_1_17_Target> targets;

  Ais8_1_17(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais8_1_17 &msg);

// No 8_1_18

// IMO Circ 289 Marine traffic signal
class Ais8_1_19 : public Ais8 {
 public:
  int link_id;
  string name;
  float x, y;  // funny bit count
  int status;
  int signal;
  int utc_hour_next, utc_min_next;
  int next_signal;
  int spare2[4];

  Ais8_1_19(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais8_1_19 &msg);

// No message 8_1_20

// IMO Circ 289 Weather observation report from ship
class Ais8_1_21 : public Ais8 {
 public:
  int type_wx_report;

  // TYPE 0
  string location;
  float x, y;  // 25, 24 bits
  int utc_day, utc_hour, utc_min;
  // wx - use wx[0]
  float horz_viz;  // nautical miles
  int humidity;  // %
  int wind_speed;  // ave knots
  int wind_dir;
  float pressure;  // hPa - float needed for type 1
  int pressure_tendency;
  float air_temp;  // C
  float water_temp;  // C
  int wave_period;  // s
  float wave_height;
  int wave_dir;
  float swell_height;  // m
  int swell_dir;
  int swell_period;  // s
  int spare2;

  // TYPE 1 - !@#$!!!!!
  // x,y
  int utc_month;
  // utc_day, hour, min
  int cog;
  float sog;
  int heading;  // Assume this is true degrees????
  // pressure defined in type 0
  float rel_pressure;  // 3 hour hPa
  // pressure_tendenc defined in type 0
  // wind_dir defined in type 0
  float wind_speed_ms;  // m/s
  int wind_dir_rel;
  float wind_speed_rel;  // m/s
  float wind_gust_speed;  // m/s
  int wind_gust_dir;
  int air_temp_raw;  // TODO(schwehr): Convert this to C.  Kelvin makes no sense
  // humidity defined in type 0
  // sea_temp_k
  int water_temp_raw;  // TODO(schwehr): fix this
  // hor_viz
  int wx[3];  // current, past 1, past 2
  int cloud_total;
  int cloud_low;
  int cloud_low_type;
  int cloud_middle_type;
  int cloud_high_type;
  float alt_lowest_cloud_base;
  // wave_period
  // wave_height
  // swell_dir
  // swell_period
  // swell_height
  int swell_dir_2, swell_period_2, swell_height_2;
  float ice_thickness;  // network is cm, storing m
  int ice_accretion;
  int ice_accretion_cause;
  int sea_ice_concentration;
  int amt_type_ice;
  int ice_situation;
  int ice_devel;
  int bearing_ice_edge;

  Ais8_1_21(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais8_1_21 &msg);

// SEE ais8_001_22.h for Area notice

// No message 8_1_23

// IMO Circ 289 Extended ship static and voyage-related
class Ais8_1_24 : public Ais8 {
 public:
  int link_id;
  float air_draught;  // m
  string last_port, next_ports[2];

  // TODO(schwehr): enum list of param types
  int solas_status[26];  // 0 NA, 1 operational, 2 SNAFU, 3 no data
  int ice_class;
  int shaft_power;  // horses
  int vhf;
  string lloyds_ship_type;
  int gross_tonnage;
  int laden_ballast;
  int heavy_oil, light_oil, diesel;
  int bunker_oil;  // tonnes
  int persons;
  int spare2;

  Ais8_1_24(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais8_1_24 &msg);

// No message 8_1_25

const size_t AIS8_1_26_REPORT_SIZE = 112;

enum Ais8_1_26_SensorEnum {
  AIS8_1_26_SENSOR_ERROR = -1,
  AIS8_1_26_SENSOR_LOCATION = 0,
  AIS8_1_26_SENSOR_STATION = 1,
  AIS8_1_26_SENSOR_WIND = 2,
  AIS8_1_26_SENSOR_WATER_LEVEL = 3,
  AIS8_1_26_SENSOR_CURR_2D = 4,
  AIS8_1_26_SENSOR_CURR_3D = 5,
  AIS8_1_26_SENSOR_HORZ_FLOW = 6,
  AIS8_1_26_SENSOR_SEA_STATE = 7,
  AIS8_1_26_SENSOR_SALINITY = 8,
  AIS8_1_26_SENSOR_WX = 9,
  AIS8_1_26_SENSOR_AIR_DRAUGHT = 10,
  AIS8_1_26_SENSOR_RESERVED_11 = 11,
  AIS8_1_26_SENSOR_RESERVED_12 = 12,
  AIS8_1_26_SENSOR_RESERVED_13 = 13,
  AIS8_1_26_SENSOR_RESERVED_14 = 14,
  AIS8_1_26_SENSOR_RESERVED_15 = 15,
};

class Ais8_1_26_SensorReport {
 public:
  int report_type;
  int utc_day, utc_hr, utc_min;
  int site_id;  // aka link_id

  virtual Ais8_1_26_SensorEnum getType() const = 0;
  virtual ~Ais8_1_26_SensorReport() {}
};

Ais8_1_26_SensorReport*
ais8_1_26_sensor_report_factory(const bitset<AIS8_MAX_BITS> &bs,
                                const size_t offset);

class Ais8_1_26_Location : public Ais8_1_26_SensorReport {
 public:
  float x, y, z;  // lon, lat, alt in m from MSL
  int owner, timeout;
  int spare;

  Ais8_1_26_Location(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_Location() {}
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_LOCATION;}
};

class Ais8_1_26_Station : public Ais8_1_26_SensorReport {
 public:
  string name;
  int spare;

  Ais8_1_26_Station(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_Station() {}
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_STATION;}
};

class Ais8_1_26_Wind : public Ais8_1_26_SensorReport {
 public:
  int wind_speed, wind_gust;  // knots
  int wind_dir, wind_gust_dir;
  int sensor_type;
  int wind_forcast, wind_gust_forcast;  // knots
  int wind_dir_forcast;
  int utc_day_forcast, utc_hour_forcast, utc_min_forcast;
  int duration;
  int spare;

  Ais8_1_26_Wind(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_Wind() {}
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_WIND;}
};

class Ais8_1_26_WaterLevel : public Ais8_1_26_SensorReport {
 public:
  int type;
  float level;  // m.  assuming it is being stored at 0.01 m inc.
  int trend;
  int vdatum;
  int sensor_type;
  int forcast_type;
  float level_forcast;
  int utc_day_forcast;
  int utc_hour_forcast;
  int utc_min_forcast;
  int duration;  // minutes
  int spare;

  Ais8_1_26_WaterLevel(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_WaterLevel() {}
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_WATER_LEVEL;}
};

struct Ais8_1_26_Curr2D_Current {
  float speed;  // knots
  int dir;
  int depth;  // m
};

class Ais8_1_26_Curr2D : public Ais8_1_26_SensorReport {
 public:
  Ais8_1_26_Curr2D_Current currents[3];
  int type;
  int spare;

  Ais8_1_26_Curr2D(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_Curr2D() {}
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_CURR_2D;}
};

struct Ais8_1_26_Curr3D_Current {
  float north, east, up;
  int depth;  // m
};

class Ais8_1_26_Curr3D : public Ais8_1_26_SensorReport {
 public:
  Ais8_1_26_Curr3D_Current currents[2];
  int type;
  int spare;

  Ais8_1_26_Curr3D(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_Curr3D() {}
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_CURR_3D;}
};

struct Ais8_1_26_HorzFlow_Current {
  int bearing, dist;  // deg, m
  float speed;  // knots
  int dir, level;  // deg, m
};

class Ais8_1_26_HorzFlow : public Ais8_1_26_SensorReport {
 public:
  Ais8_1_26_HorzFlow_Current currents[2];
  int spare;

  Ais8_1_26_HorzFlow(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_HorzFlow() {}
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_HORZ_FLOW;}
};

class Ais8_1_26_SeaState : public Ais8_1_26_SensorReport {
 public:
  float swell_height;
  int swell_period, swell_dir;  // s, deg
  int sea_state;
  int swell_sensor_type;
  float water_temp, water_temp_depth;  // C, m
  int water_sensor_type;
  float wave_height;
  int wave_period, wave_dir;  // s, deg
  int wave_sensor_type;
  float salinity;

  Ais8_1_26_SeaState(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_SeaState() {}
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_SEA_STATE;}
};

class Ais8_1_26_Salinity : public Ais8_1_26_SensorReport {
 public:
  float water_temp;  // C
  float conductivity;  // siemens/m
  float pressure;  // decibars
  float salinity;  // 0/00 ppt
  int salinity_type;
  int sensor_type;
  int spare[2];

  Ais8_1_26_Salinity(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_Salinity() {}
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_SALINITY;}
};

class Ais8_1_26_Wx : public Ais8_1_26_SensorReport {
 public:
  float air_temp;  // C
  int air_temp_sensor_type;
  int precip;
  float horz_vis;  // nm
  float dew_point;  // C
  int dew_point_type;
  int air_pressure;  // hPa
  int air_pressure_trend;
  int air_pressor_type;
  float salinity;  // 0/00 ppt
  int spare;

  Ais8_1_26_Wx(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_Wx() {}
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_WX;}
};

class Ais8_1_26_AirDraught : public Ais8_1_26_SensorReport {
 public:
  float draught, gap, forcast_gap;
  int trend;
  int utc_day_forcast, utc_hour_forcast, utc_min_forcast;
  int spare;

  Ais8_1_26_AirDraught(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_AirDraught() {}
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_AIR_DRAUGHT;}
};

// IMO Circ 289 Environmental
class Ais8_1_26 : public Ais8 {
 public:
  vector<Ais8_1_26_SensorReport *> reports;

  Ais8_1_26(const char *nmea_payload, const size_t pad);
  ~Ais8_1_26();
};
ostream& operator<< (ostream &o, const Ais8_1_26 &msg);


// IMO Circ 289 Route information
class Ais8_1_27 : public Ais8 {
 public:
  int link_id;
  int sender_type, route_type;
  int utc_month, utc_day, utc_hour, utc_min;
  int duration;
  vector<AisPoint> waypoints;

  Ais8_1_27(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais8_1_27 &msg);


//  No message 8_1_28


// IMO Circ 289 Text description
class Ais8_1_29 : public Ais8 {
 public:
  int link_id;
  string text;
  int spare2;

  Ais8_1_29(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais8_1_29 &msg);


// No message 8_1_30

// IMO Circ 289 Meteorological and Hydrographic data
// Section 1, Table 1.1
// TODO(schwehr): is this exactly the same as 8_1_11 or has anything changed?
//       x,y swapped.
class Ais8_1_31 : public Ais8 {
 public:
  float x, y;  // Opposite the bit order of 8_1_11
  int position_accuracy;  // New field
  int utc_day;
  int utc_hour;
  int utc_min;
  int wind_ave;  // kts
  int wind_gust;  // kts
  int wind_dir;
  int wind_gust_dir;
  float air_temp;  // C
  int rel_humid;
  float dew_point;
  int air_pres;
  int air_pres_trend;
  float horz_vis;  // NM
  float water_level;  // m
  int water_level_trend;

  float surf_cur_speed;
  int surf_cur_dir;
  float cur_speed_2;  // kts
  int cur_dir_2;
  int cur_depth_2;  // m
  float cur_speed_3;  // kts
  int cur_dir_3;
  int cur_depth_3;  // m

  float wave_height;  // m
  int wave_period;
  int wave_dir;
  float swell_height;  // m
  int swell_period;
  int swell_dir;
  int sea_state;  // beaufort scale - Table 1.2
  float water_temp;
  int precip_type;
  float salinity;
  int ice;  // yes/no/undef/unknown
  int spare2;

  Ais8_1_31(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais8_1_31 &msg);

// ECE-TRANS-SC3-2006-10e-RIS.pdf - River Information System
// Inland ship static and voyage related data
class Ais8_200_10 : public Ais8 {
 public:
  string eu_id;  // European Vessel ID - 8 characters
  float length, beam;  // m
  int ship_type;
  int haz_cargo;
  float draught;
  int loaded;
  int speed_qual, course_qual, heading_qual;  // sensor quality
  int spare2;

  Ais8_200_10(const char *nmea_payload, const size_t pad);
};

// 21 and 22 do not exist

// ECE-TRANS-SC3-2006-10e-RIS.pdf - River Information System
class Ais8_200_23 : public Ais8 {
 public:
  int utc_year_start, utc_month_start, utc_day_start;
  int utc_year_end, utc_month_end, utc_day_end;
  int utc_hour_start, utc_min_start;
  int utc_hour_end, utc_min_end;
  float x1, y1;
  float x2, y2;
  int type;
  int min;
  int max;
  int classification;
  int wind_dir;  // EMMA CODE
  int spare2;

  Ais8_200_23(const char *nmea_payload, const size_t pad);
};

// ECE-TRANS-SC3-2006-10e-RIS.pdf - River Information System
// Water Level
class Ais8_200_24 : public Ais8 {
 public:
  string country;
  int guage_ids[4];
  float levels[4];  // m

  Ais8_200_24(const char *nmea_payload, const size_t pad);
};

// ECE-TRANS-SC3-2006-10e-RIS.pdf - River Information System
class Ais8_200_40 : public Ais8 {
 public:
  float x, y;
  int form;
  int dir;  // degrees
  int stream_dir;
  int status_raw;
  // TODO(schwehr): int status[9];  // WTF is the encoding for this?
  int spare2;

  Ais8_200_40(const char *nmea_payload, const size_t pad);
};

// ECE-TRANS-SC3-2006-10e-RIS.pdf - River Information System
class Ais8_200_55 : public Ais8 {
 public:
  int crew;
  int passengers;
  int yet_more_personnel;  // WTF?  Like a maid or waiter?
  int spare2[3];  // JERKS... why 51 spare bits?

  Ais8_200_55(const char *nmea_payload, const size_t pad);
};

enum Ais8_366_22_AreaShapeEnum {
    AIS8_366_22_SHAPE_ERROR = -1,
    AIS8_366_22_SHAPE_CIRCLE = 0,
    AIS8_366_22_SHAPE_RECT = 1,
    AIS8_366_22_SHAPE_SECTOR = 2,
    AIS8_366_22_SHAPE_POLYLINE = 3,
    AIS8_366_22_SHAPE_POLYGON = 4,
    AIS8_366_22_SHAPE_TEXT = 5,
    AIS8_366_22_SHAPE_RESERVED_6 = 6,
    AIS8_366_22_SHAPE_RESERVED_7 = 7
};

extern const char *shape_names[8];

class Ais8_366_22_SubArea {
 public:
    virtual Ais8_366_22_AreaShapeEnum getType()=0;
    virtual ~Ais8_366_22_SubArea() { }
};

Ais8_366_22_SubArea*
ais8_366_22_subarea_factory(const bitset<AIS8_MAX_BITS> &bs,
                            const size_t offset);

// or Point if radius is 0
class Ais8_366_22_Circle : public Ais8_366_22_SubArea {
 public:
    float x, y;
    // TODO(schwehr): int precision
    int radius_m;
    unsigned int spare;

    Ais8_366_22_Circle(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
    ~Ais8_366_22_Circle() {}
    Ais8_366_22_AreaShapeEnum getType() {return AIS8_366_22_SHAPE_CIRCLE;}
};

class Ais8_366_22_Rect : public Ais8_366_22_SubArea {
 public:
    float x, y;  // longitude and latitude
    // TODO(schwehr): int precision
    int e_dim_m;  // East dimension in meters
    int n_dim_m;
    int orient_deg;  // Orientation in degrees from true north
    unsigned int spare;  // 5 bits

    Ais8_366_22_Rect(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
    ~Ais8_366_22_Rect() {}
    Ais8_366_22_AreaShapeEnum getType() {return AIS8_366_22_SHAPE_RECT;}
};

class Ais8_366_22_Sector : public Ais8_366_22_SubArea {
 public:
    float x, y;
    // TODO(schwehr): int precision
    int radius_m;
    int left_bound_deg;
    int right_bound_deg;
    // TODO(schwehr): spare?

    Ais8_366_22_Sector(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
    ~Ais8_366_22_Sector() {}
    Ais8_366_22_AreaShapeEnum getType() {return AIS8_366_22_SHAPE_SECTOR;}
};

// Or Waypoint
// Must have a point before on the VDL, but pulled together here.
class Ais8_366_22_Polyline : public Ais8_366_22_SubArea {
 public:
    float x, y;  // longitude and latitude
    // TODO(schwehr): precision

    // Up to 4 points
    vector<float> angles;
    vector<float> dists_m;
    unsigned int spare;

    Ais8_366_22_Polyline(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
    ~Ais8_366_22_Polyline() {}
    Ais8_366_22_AreaShapeEnum getType() {return AIS8_366_22_SHAPE_POLYLINE;}
};

class Ais8_366_22_Polygon : public Ais8_366_22_SubArea {
 public:
    float x, y;  // longitude and latitude
    // TODO(schwehr): precision?

    // Up to 4 points in a first message, but aggregated if multiple sub areas
    vector<float> angles;
    vector<float> dists_m;
    unsigned int spare;

    Ais8_366_22_Polygon(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
    ~Ais8_366_22_Polygon() {}
    Ais8_366_22_AreaShapeEnum getType() {return AIS8_366_22_SHAPE_POLYGON;}
};

class Ais8_366_22_Text : public Ais8_366_22_SubArea {
 public:
    string text;
    unsigned int spare;  // 3 bits

    Ais8_366_22_Text(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
    ~Ais8_366_22_Text() {}
    Ais8_366_22_AreaShapeEnum getType() {return AIS8_366_22_SHAPE_TEXT;}
};

class Ais8_366_22 : public Ais8 {
 public:
    // Common block at the front
    int link_id;  // 10 bit id to match up text blocks
    int notice_type;  // area_type / Notice Description
    int month;  // These really are in utc
    int day;
    int utc_hour;
    int utc_minute;
    int duration_minutes;  // Time from the start until the notice expires
    // 1 or more sub messages

    vector<Ais8_366_22_SubArea *> sub_areas;

  Ais8_366_22(const char *nmea_payload, const size_t pad);
    ~Ais8_366_22();
};
ostream& operator<< (ostream& o, Ais8_366_22 const& msg);

const size_t AIS8_366_22_NUM_NAMES = 128;
extern const char *ais8_366_22_notice_names[AIS8_366_22_NUM_NAMES];

// 366 34 - Kurt older whale message 2008-2010
// TODO(schwehr): Ais8_366_34

class Ais9 : public AisMsg {
 public:
  int alt;  // m above sea level
  float sog;
  int position_accuracy;
  float x, y;
  float cog;
  int timestamp;
  int alt_sensor;
  int spare;
  int dte;
  int spare2;
  int assigned_mode;
  bool raim;
  int commstate_flag;

  int sync_state;  // In both SOTDMA and ITDMA

  // SOTDMA
  int slot_timeout;
  bool slot_timeout_valid;

  // Based on slot_timeout which ones are valid
  int received_stations;
  bool received_stations_valid;

  int slot_number;
  bool slot_number_valid;

  bool utc_valid;
  int utc_hour;
  int utc_min;
  int utc_spare;

  int slot_offset;
  bool slot_offset_valid;

  // ITDMA
  int slot_increment;
  bool slot_increment_valid;

  int slots_to_allocate;
  bool slots_to_allocate_valid;

  bool keep_flag;
  bool keep_flag_valid;

  Ais9(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais9 &msg);

// 10 ":" - UTC and date inquiry
class Ais10 : public AisMsg {
 public:
  int spare;
  int dest_mmsi;
  int spare2;

  Ais10(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais10 &msg);

// 11 ';' - See 4_11

// '<' - Addressd safety related
class Ais12 : public AisMsg {
 public:
  int seq_num;
  int dest_mmsi;
  bool retransmitted;
  int spare;
  string text;

  Ais12(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais12 &msg);

// 13 '=' - See 7

// '>' - Safety broadcast
class Ais14 : public AisMsg {
 public:
  int spare;
  string text;
  int expected_num_spare_bits;  // The bits in the nmea_payload not used

  Ais14(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais14 &msg);

// ? - Interrogation
class Ais15 : public AisMsg {
 public:
  int spare;
  int mmsi_1;
  int msg_1_1;
  int slot_offset_1_1;

  int spare2;
  int dest_msg_1_2;
  int slot_offset_1_2;

  int spare3;
  int mmsi_2;
  int msg_2;
  int slot_offset_2;
  int spare4;

  Ais15(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais15 &msg);

// @ - Assigned mode command
class Ais16 : public AisMsg {
 public:
  int spare;
  int dest_mmsi_a;
  int offset_a;
  int inc_a;
  int dest_mmsi_b;
  int offset_b;
  int inc_b;
  int spare2;

  Ais16(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais16 &msg);

// ITU-R M.823  http://www.itu.int/rec/R-REC-M.823/en
// A - GNSS broacast - TODO(schwehr): only partially coded
class Ais17 : public AisMsg {
 public:
  int spare;
  float x, y;
  int spare2;
  int gnss_type;
  int z_cnt;
  int station;
  int seq;
  // N - do not need to store this
  int health;
  // TODO(schwehr): Handle payload

  Ais17(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais17 &msg);


// B - Class B position report
class Ais18 : public AisMsg {
 public:
  int spare;
  float sog;
  int position_accuracy;
  float x, y;  // Long and lat
  float cog;
  int true_heading;
  int timestamp;
  int spare2;
  int unit_flag;
  int display_flag;
  int dsc_flag;
  int band_flag;
  int m22_flag;
  int mode_flag;
  bool raim;
  int commstate_flag;

  // SOTDMA
  int sync_state;
  int slot_timeout;

  // Based on slot_timeout which ones are valid
  int received_stations;
  bool received_stations_valid;

  int slot_number;
  bool slot_number_valid;

  bool utc_valid;
  int utc_hour;
  int utc_min;
  int utc_spare;

  int slot_offset;
  bool slot_offset_valid;

  // ITDMA
  int slot_increment;
  bool slot_increment_valid;

  int slots_to_allocate;
  bool slots_to_allocate_valid;

  bool keep_flag;
  bool keep_flag_valid;

  Ais18(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais18 &msg);

// C - Class B extended ship and position
class Ais19 : public AisMsg {
 public:
  int spare;
  float sog;
  int position_accuracy;
  float x, y;  // Long and lat
  float cog;
  int true_heading;
  int timestamp;
  int spare2;
  string name;
  int type_and_cargo;
  int dim_a;
  int dim_b;
  int dim_c;
  int dim_d;
  int fix_type;
  bool raim;
  int dte;
  int assigned_mode;
  int spare3;

  Ais19(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais19 &msg);

// 'D' - Data link management
// TODO(schwehr): consider a vector
class Ais20 : public AisMsg {
 public:
  int spare;
  int offset_1;
  int num_slots_1;
  int timeout_1;
  int incr_1;

  int offset_2;
  int num_slots_2;
  int timeout_2;
  int incr_2;
  bool group_valid_2;

  int offset_3;
  int num_slots_3;
  int timeout_3;
  int incr_3;
  bool group_valid_3;

  int offset_4;
  int num_slots_4;
  int timeout_4;
  int incr_4;
  bool group_valid_4;
  int spare2;

  Ais20(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais20 &msg);

// 'E' - Aids to navigation report
class Ais21 : public AisMsg {
 public:
  int aton_type;
  string name;
  int position_accuracy;
  float x, y;
  int dim_a;
  int dim_b;
  int dim_c;
  int dim_d;
  int fix_type;
  int timestamp;
  bool off_pos;
  int aton_status;
  bool raim;
  bool virtual_aton;
  bool assigned_mode;
  int spare;
  // Extended name goes on the end of name
  int spare2;

  Ais21(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais21 &msg);

// 'F' - Channel Management
class Ais22 : public AisMsg {
 public:
  int spare;
  int chan_a;
  int chan_b;
  int txrx_mode;
  bool power_low;

  // if addressed false, then geographic position
  bool pos_valid;
  float x1, y1;
  float x2, y2;

  // if addressed is true
  bool dest_valid;
  int dest_mmsi_1;
  int dest_mmsi_2;

  int chan_a_bandwidth;
  int chan_b_bandwidth;
  int zone_size;

  int spare2;  // Lame that they make a huge spare here.  Bad bad bad

  Ais22(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais22 &msg);

// 'G' - Group Assignment Command
class Ais23 : public AisMsg {
 public:
  int spare;
  float x1, y1;
  float x2, y2;
  int station_type;
  int type_and_cargo;

  int spare2;  // 22 bits of spare here?  what were people thinking?

  int txrx_mode;
  int interval_raw;  // raw value, not sec
  int quiet;
  int spare3;

  Ais23(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais23 &msg);

// Class B Static Data report
class Ais24 : public AisMsg {
 public:
  int part_num;

  // Part A
  string name;

  // Part B
  int type_and_cargo;
  string vendor_id;
  string callsign;
  int dim_a;
  int dim_b;
  int dim_c;
  int dim_d;
  int spare;

  Ais24(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais24 &msg);

// 'I' - Single slot binary message - addressed or broadcast
// TODO(schwehr): handle payload
class Ais25 : public AisMsg {
 public:
  bool use_app_id;  // if false, payload is unstructured binary.

  bool dest_mmsi_valid;
  int dest_mmsi;  // only valid if addressed
  // If unstructured:
  // TODO(schwehr): vector<unsigned char> payload;

  int dac;  // valid if use_app_id is true
  int fi;

  Ais25(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais25 &msg);

// 'J' - Multi slot binary message with comm state
// TODO(schwehr): handle payload
class Ais26 : public AisMsg {
 public:
  bool use_app_id;  // if false, payload is unstructured binary

  bool dest_mmsi_valid;
  int dest_mmsi;  // only valid if addressed

  int dac;  // valid it use_app_id
  int fi;

  // TODO(schwehr): vector<unsigned char> payload;  // If unstructured.  Yuck.

  int commstate_flag;  // 0 - SOTDMA, 1 - TDMA

  // SOTDMA
  int sync_state;
  int slot_timeout;
  bool slot_timeout_valid;

  // Based on slot_timeout which ones are valid
  int received_stations;
  bool received_stations_valid;

  int slot_number;
  bool slot_number_valid;

  bool utc_valid;
  int utc_hour;
  int utc_min;
  int utc_spare;

  int slot_offset;
  bool slot_offset_valid;

  // ITDMA
  int slot_increment;
  bool slot_increment_valid;

  int slots_to_allocate;
  bool slots_to_allocate_valid;

  bool keep_flag;
  bool keep_flag_valid;

  Ais26(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais26 &msg);

// K - Long-range position report - e.g. for satellite receivers
class Ais27 : public AisMsg {
 public:
  int position_accuracy;
  bool raim;
  int nav_status;
  float x, y;
  int sog;
  int cog;
  bool gnss;  // warning: bits in AIS are flipped sense
  int spare;

  Ais27(const char *nmea_payload, const size_t pad);
};
ostream& operator<< (ostream &o, const Ais27 &msg);

//////////////////////////////////////////////////////////////////////
// Support templates for decoding
//////////////////////////////////////////////////////////////////////

extern bitset<6> nmea_ord[128];

template<size_t T>
AIS_STATUS aivdm_to_bits(bitset<T> &bits, const char *nmea_payload) {
  assert(nmea_payload);
  if (strlen(nmea_payload) > T/6) {
#ifndef NDEBUG
    std::cerr << "ERROR: message longer than max allowed size (" << T/6
              << "): found " << strlen(nmea_payload) << " characters in "
              << nmea_payload << std::endl;
#endif
    return AIS_ERR_MSG_TOO_LONG;
  }
  for (size_t idx = 0; nmea_payload[idx] != '\0' && idx < T/6; idx++) {
    int c = static_cast<int>(nmea_payload[idx]);
    if (c < 48 || c > 119 || (c >= 88 && c <= 95)) {
      return AIS_ERR_BAD_NMEA_CHR;
    }
    const bitset<6> bs_for_char = nmea_ord[ c ];
    for (size_t offset = 0; offset < 6; offset++) {
      bits[idx*6 + offset] = bs_for_char[offset];
    }
  }
  return AIS_OK;
}

// TODO(schwehr): turn ubits, sbits, and ais_str into a helper class.
template<size_t T>
int ubits(const bitset<T> &bits, const size_t start, const size_t len) {
  assert(len <= 32);
  assert(start + len <= T);
  bitset<32> bs_tmp;
  for (size_t i = 0; i < len; i++)
    bs_tmp[i] = bits[start + len - i - 1];
  return bs_tmp.to_ulong();
}

// TODO(schwehr): do not use long
typedef union {
  long long_val;
  unsigned long ulong_val;
} long_union;

template<size_t T>
int sbits(bitset<T> bs, const size_t start, const size_t len) {
  assert(len <= 32);
  assert(start + len <= T);  // TODO(schwehr):  should it just be < ?
  bitset<32> bs32;
  // pad 1's to the left if negative
  if (len < 32 && 1 == bs[start] ) bs32.flip();

  for (size_t i = 0; i < len; i++)
    bs32[i] = bs[start + len - i - 1];

  long_union val;
  val.ulong_val = bs32.to_ulong();
  return val.long_val;
}

extern const string bits_to_char_tbl;

template<size_t T>
const string ais_str(const bitset<T> &bits, const size_t start,
                     const size_t len) {
  assert(start + len < T);
  assert(len % 6 == 0);
  const size_t num_char = len / 6;
  string result(num_char, '@');
  for (size_t char_idx = 0; char_idx < num_char; char_idx++) {
    const int char_num = ubits(bits, start + char_idx*6, 6);
    result[char_idx] = bits_to_char_tbl[char_num];
  }
  return result;
}

#endif
