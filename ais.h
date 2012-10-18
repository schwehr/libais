#ifndef AIS_H
#define AIS_H

// -*- c++ -*-
// Since Apr 2010
// g++ ais_pos.cxx -o ais_pos -g -Wall -O3 -Wimplicit -W -Wredundant-decls -pedantic  -funroll-loops -fexpensive-optimizations

#include <bitset>
#include <string>
#include <cassert>
#include <vector>
#include <cstring>
#include <string>

#include <iostream> // for checkpoint

//#define LIBAIS_VERSION_MAJOR @LIBAIS_VERSION_MAJOR@
//#define LIBAIS_VERSION_MINOR @LIBAIS_VERSION_MINOR@
#define LIBAIS_VERSION_MAJOR 0
#define LIBAIS_VERSION_MINOR 8

//////////////////////////////////////////////////////////////////////
// Helpers

#define CHECKPOINT std::cerr <<  __FILE__ << ": line " << __LINE__ << ": checkpoint" << std::endl

const std::string nth_field(const std::string &str, const size_t n, const char c);

//////////////////////////////////////////////////////////////////////

extern bool nmea_ord_initialized; // If this is false, you need to call build_nmea_lookup.

void build_nmea_lookup();

//void aivdm_to_bits(bitset<168> &bits, const char *nmea_payload);

enum AIS_STATUS {
    AIS_OK,
    AIS_ERR_BAD_BIT_COUNT,
    AIS_ERR_WRONG_MSG_TYPE,
    AIS_ERR_BAD_NMEA_CHR,
    AIS_ERR_BAD_PTR,
    AIS_ERR_UNKNOWN_MSG_TYPE,
    AIS_ERR_MSG_NOT_IMPLEMENTED, // Meaning I haven't got to it yet
    AIS_ERR_EXPECTED_STRING,
    AIS_ERR_BAD_MSG_CONTENT,
    AIS_ERR_MSG_TOO_LONG,
    AIS_STATUS_NUM_CODES
};

extern const char *const AIS_STATUS_STRINGS[AIS_STATUS_NUM_CODES];

struct AisPoint {
  float x,y;
};

class AisMsg {
public:
    int message_id;
    int repeat_indicator;
    int mmsi;

    bool had_error() {
        /*
        std::cout << "in_had_error: " << status << " " << AIS_OK << " "  << (AIS_OK != status)
                  << " " << false << true << std::endl
                  << "\t" << AIS_STATUS_STRINGS[AIS_OK] << " "
                  <<  AIS_STATUS_STRINGS[status] << "\n";

        const bool result = (status != AIS_OK);
        std::cout << "error_status: " << (result?"HAD_ERROR":"should be okay") << std::endl;
        return result; */
        return status != AIS_OK;
    }
    AIS_STATUS get_error() {return status;}
    AIS_STATUS status; // AIS_OK or error code
    //AisMsg() {init();}
//protected:
    void init() {
        //std::cout << "AisMsg_init: setting ok" << std::endl;
        status = AIS_OK;
#ifndef NDEBUG
        // FIX: should we be setting these?  The individual messages need to do this.
        message_id = repeat_indicator = mmsi = -666;
#endif
    }
};

class Ais1_2_3 : public AisMsg {
public:
    int nav_status;
    bool rot_over_range;
    int rot_raw;
    float rot;
    float sog;
    int position_accuracy;
    float x, y; // Long and lat
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

    bool utc_valid;  // Only means that the values are set.  Can still have
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

    Ais1_2_3(const char *nmea_payload);
    void print(bool verbose=false);
};
std::ostream& operator<< (std::ostream& o, Ais1_2_3 const& a);

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
    float x, y; // longitude, latitude
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

    bool utc_valid;  // Only means that the values are set.  Can still have
    int utc_hour;
    int utc_min;
    int utc_spare;

    int slot_offset;
    bool slot_offset_valid;

    // **NO** ITDMA
    Ais4_11(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais4_11 const& msg);


class Ais5 : public AisMsg {
 public:
    int ais_version;
    int imo_num;
    std::string callsign;
    std::string name;
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
    float draught; // present static draft. m
    std::string destination;
    int dte;
    int spare;

    Ais5(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais5 const& msg);

// Addessed binary message (ABM)
const size_t AIS6_MAX_BITS = 1192;

// AIS Binary Broadcast message ... parent to many
class Ais6 : public AisMsg {
public:
    Ais6() {}

    static const int MAX_BITS = AIS6_MAX_BITS;

    int seq; // sequence number
    int mmsi_dest;
    bool retransmit;
    int spare;
    int dac; // dac+fi = app id
    int fi;

    std::vector<unsigned char> payload; // If dac/fi (app id is now one we know).  without dac/fi

  Ais6(const char *nmea_payload, const size_t pad);
  //bool decode_header6(const std::bitset<MAX_BITS> &bs);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais6 const& msg);

#if 0
template<size_t T>
bool decode_header6(std::bitset<T> &bs, Ais6 &msg) {
  msg.message_id = ubits(bs, 0, 6);
  if (6 != msg.message_id) { msg.status = AIS_ERR_WRONG_MSG_TYPE; return false; }
  msg.repeat_indicator = ubits(bs,6,2);
  msg.mmsi = ubits(bs,8,30);
  msg.seq = ubits(bs,38,2);
  msg.mmsi_dest = ubits(bs, 40, 30);
  msg.retransmit = !bool(bs[70]);
  msg.spare = bs[71];
  msg.dac = ubits(bs,72,10);
  msg.fi = ubits(bs,82,6);
}
#endif

// Text message - ITU 1371-1 - this is OLD
class Ais6_1_0 : public Ais6 {
 public:
  bool ack_required;
  int msg_seq;
  std::string text;
  int spare2;

  Ais6_1_0(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais6_1_0 const& msg);

// ITU 1371-1 - this is OLD
class Ais6_1_1 : public Ais6 {
 public:
  int ack_dac;
  int msg_seq;
  int spare2;

  Ais6_1_1(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais6_1_1 const& msg);


// ITU 1371-1 - this is OLD
class Ais6_1_2 : public Ais6 {
 public:
  int req_dac;
  int req_fi;
  //int spare2;

  Ais6_1_2(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais6_1_2 const& msg);


// capability interogation - ITU 1371-1 - this is OLD
class Ais6_1_3 : public Ais6 {
 public:
  int req_dac;
  int spare2;

  Ais6_1_3(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais6_1_3 const& msg);


// Capability interogation reply - ITU 1371-1 - this is OLD
class Ais6_1_4 : public Ais6 {
 public:
  int ack_dac;
  int capabilities[64];
  int cap_reserved[64];
  int spare2;

  Ais6_1_4(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais6_1_4 const& msg);

// Number of Persons ITU 1371-1 - this is OLD
class Ais6_1_40 : public Ais6 {
 public:
  int persons;
  int spare2;

  Ais6_1_40(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais6_1_40 const& msg);


// IMO Circ 236 Dangerous cargo indication - Not to be transmitted after 2012-Jan-01
class Ais6_1_12 : public Ais6 {
 public:
  std::string last_port;
  int utc_month_dep; // actual time of departure
  int utc_day_dep, utc_hour_dep, utc_min_dep;
  std::string next_port;
  int utc_month_next; // estimated arrival
  int utc_day_next, utc_hour_next, utc_min_next;
  std::string main_danger;
  std::string imo_cat;
  int un;
  int value; // UNIT???
  int value_unit;
  int spare2;

  Ais6_1_12(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais6_1_12 const& msg);


struct Ais6_1_14_Window {
  float y, x;  // yes, bits are lat, lon
  int utc_hour_from, utc_min_from;
  int utc_hour_to, utc_min_to;
  int cur_dir;
  float cur_speed;
};


// IMO Circ 236 Tidal window - Not to be transmitted after 2012-Jan-01
class Ais6_1_14 : public Ais6 {
 public:
  int utc_month, utc_day;
  std::vector<Ais6_1_14_Window> windows;

  Ais6_1_14(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais6_1_14 const& msg);


// IMO Circ 289 Clearance time to enter port
class Ais6_1_18 : public Ais6 {
 public:
  int link_id;
  int utc_month, utc_day, utc_hour, utc_min;
  std::string port_berth, dest;
  float x, y;
  int spare2[2]; // 32 bits per spare

  Ais6_1_18(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais6_1_18 const& msg);


// IMO Circ 289 Berthing data
class Ais6_1_20 : public Ais6 {
 public:
  int link_id;
  int length;
  float depth;
  int position;
  int utc_month, utc_day, utc_hour, utc_min;
  bool services_known;
  // TODO: enum of service types
  int services[26];
  std::string name;
  float x, y;
  //int spare;  No bits?  WTF

  Ais6_1_20(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais6_1_20 const& msg);


struct Ais6_1_25_Cargo {
  int code_type;
  bool imdg_valid; // also set with BC
  int imdg;
  bool spare_valid;
  int spare; // imdg or dc or marpols
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

  std::vector<Ais6_1_25_Cargo> cargos;  // 0 to 17 cargo entries

  Ais6_1_25(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais6_1_25 const& msg);



// IMO Circ 289 Route information
class Ais6_1_28 : public Ais6 {
 public:
  int link_id;
  int sender_type, route_type;
  int utc_month_start, utc_day_start, utc_hour_start, utc_min_start;
  int duration;
  std::vector<AisPoint> waypoints;

  Ais6_1_28(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais6_1_28 const& msg);


// IMO Circ 289 Text description
class Ais6_1_30 : public Ais6 {
 public:
  int link_id;
  std::string text;

  Ais6_1_30(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais6_1_30 const& msg);


// IMO Circ 289
// could use the same struct for for 8_1_14, but x and y are bit wise different.
// two structs hints that things are not the same bitwise.
struct Ais6_1_32_Window {
  float x,y;
  int from_utc_hour, from_utc_min;
  int to_utc_hour, to_utc_min;
  int cur_dir;
  float cur_speed; // knots
};

// IMO Circ 289 Tidal window
class Ais6_1_32 : public Ais6 {
 public:
  int utc_month;
  int utc_day;

  std::vector<Ais6_1_32_Window> windows;

  Ais6_1_32(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais6_1_32 const& msg);


//////////////////////////////////////////////////////////////////////

// 7 and 13 are ACKs for msg 6 and 12
class Ais7_13 : public AisMsg {
public:
    int spare;

    std::vector<int> dest_mmsi;
    std::vector<int> seq_num;

    Ais7_13(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais7_13 const& msg);

// 366 34 - Kurt older whale message 2008-2010

const size_t AIS8_MAX_BITS = 1192;

// AIS Binary Broadcast message ... parent to many
class Ais8 : public AisMsg {
public:
  Ais8() {}

  static const int MAX_BITS = AIS8_MAX_BITS; //1192; //1008;

  int spare;
  //int seq; // ITU M.R. 1371-3 Anex 2 5.3.1
  int dac; // dac+fi = app id
  int fi;

  std::vector<unsigned char> payload; // If dac/fi (app id is now one we know).  without dac/fi

  Ais8(const char *nmea_payload);
  bool decode_header8(const std::bitset<MAX_BITS> &bs);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais8 const& msg);

// Text telegram ITU 1371-1 - this is OLD
class Ais8_1_0 : public Ais8 {
public:
  bool ack_required;
  int msg_seq;
  std::string text;
  int spare2;

  Ais8_1_0(const char *nmea_payload, size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_1_0 const& msg);

// 8_1_1 No message
// 8_1_2 No message
// 8_1_3 No message
// 8_1_4 No message

// ITU 1371-1 - this is OLD
// class Ais8_1_ : public Ais8 {
// public:

//   Ais8_1_(const char *nmea_payload, size_t pad);
//     void print();
// };
// std::ostream& operator<< (std::ostream& o, Ais8_1_ const& msg);

// Persons on board ITU 1371-1 - this is OLD
class Ais8_1_40 : public Ais8 {
public:
  int persons;
  int spare2;
  Ais8_1_40(const char *nmea_payload, size_t pad);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_1_40 const& msg);


// IMO Circ 289 met hydro - Not to be transmitted after 2013-Jan-01
// See also IMO Circ 236
class Ais8_1_11 : public Ais8 {
public:
    float x, y; // warning... appears in the bit stream as y,x
    int day;
    int hour;
    int minute;
    int wind_ave; // kts
    int wind_gust; // kts
    int wind_dir;
    int wind_gust_dir;
    float air_temp; // C
    int rel_humid;
    float dew_point;
    float air_pres;
    int air_pres_trend;
    float horz_vis; // NM
    float water_level; // m
    int water_level_trend;
    float surf_cur_speed;
    int surf_cur_dir;
    float cur_speed_2; // kts
    int cur_dir_2;
    int cur_depth_2; // m
    float cur_speed_3; // kts
    int cur_dir_3;
    int cur_depth_3; // m
    float wave_height; // m
    int wave_period;
    int wave_dir;
    float swell_height;
    int swell_period;
    int swell_dir;
    int sea_state; // beaufort scale
    float water_temp;
    int precip_type;
    float salinity;
    int ice; // yes/no/undef/unknown
    int spare2;
  int extended_water_level; //spare;  // OHMEX uses this for extra water level precision

  Ais8_1_11(const char *nmea_payload, size_t pad);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_1_11 const& msg);


// IMO Circ 236 Fairway closed - Not to be transmitted after 2012-Jan-01
class Ais8_1_13 : public Ais8 {
 public:
  std::string reason, location_from, location_to;
  int radius;
  int units;
  // utc?  warning: day/month out of order
  int day_from, month_from, hour_from, minute_from;
  int day_to, month_to, hour_to, minute_to;
  int spare2;

  Ais8_1_13(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_1_13 const& msg);


// IMO Circ 236 Extended ship static and voyage data - Not to be transmitted after 2012-Jan-01
class Ais8_1_15 : public Ais8 {
 public:
  float air_draught;
  int spare2;

  Ais8_1_15(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_1_15 const& msg);


// IMO Circ 236 Number of persons on board
class Ais8_1_16 : public Ais8 {
 public:
  int persons;
  int spare2;

  Ais8_1_16(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_1_16 const& msg);


struct Ais8_1_17_Target {
  int type;
  std::string id;
  int spare;
  float x,y; // bits are lat, lon
  int cog;
  int timestamp;
  int sog;
};

// IMO Circ 236 VTS Generated/synthetic targets
class Ais8_1_17 : public Ais8 {
 public:
  std::vector<Ais8_1_17_Target> targets;

  Ais8_1_17(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_1_17 const& msg);


// No 8_1_18


// IMO Circ 289 Marine traffic signal
class Ais8_1_19 : public Ais8 {
 public:
  int link_id;
  std::string name;
  float x, y;  // funny bit count
  int status;
  int signal;
  int utc_hour_next, utc_min_next;
  int next_signal;
  int spare2[4];  // Arrrrrr.  102 wasted bits that could be corrupted.

  Ais8_1_19(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_1_19 const& msg);

// No message 8_1_20

// IMO Circ 289 Weather observation report from ship
class Ais8_1_21 : public Ais8 {
 public:
  int type_wx_report;

  // TYPE 0
  std::string location;
  float x, y; // 25, 24 bits
  int utc_day, utc_hour, utc_min;
  //int wx;
  float horz_viz; // nautical miles
  int humidity;  // %
  int wind_speed;  // ave knots
  int wind_dir;
  float pressure; // hPa - float needed for type 1
  int pressure_tendency;
  float air_temp; // C
  float water_temp; // C
  int wave_period; // s
  float wave_height;
  int wave_dir;
  float swell_height; // m
  int swell_dir;
  int swell_period; // s
  int spare2;

  // TYPE 1 - !@#$!!!!!
  // x,y
  int utc_month;
  // utc_day, hour, min
  int cog;
  float sog;
  int heading;  // Assume this is true degrees????
  //int pressure;
  float rel_pressure;  // 3 hour hPa
  // pressure_tendency
  //int wind_dir;
  float wind_speed_ms; // m/s
  int wind_dir_rel;
  float wind_speed_rel; // m/s
  float wind_gust_speed; // m/s
  int wind_gust_dir;
  int air_temp_raw;  // Seriously?  I'm not saving air temperature in fracking kelvin.
  // humid
  // sea_temp_k
  int water_temp_raw;  // TODO fix this
  // hor_viz
  int wx[3]; // current, past 1, past 2
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
  float ice_thickness; // network is cm, storing m
  int ice_accretion;
  int ice_accretion_cause;
  int sea_ice_concentration;
  int amt_type_ice;
  int ice_situation;
  int ice_devel;
  int bearing_ice_edge;

  Ais8_1_21(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_1_21 const& msg);


// SEE ais8_001_22.h for Area notice


// No message 8_1_23


// IMO Circ 289 Extended ship static and voyage-related
class Ais8_1_24 : public Ais8 {
 public:
  int link_id;
  float air_draught;  // m
  std::string last_port, next_ports[2];

  // TODO enum list of param types
  int solas_status[26]; // 0 NA, 1 operational, 2 SNAFU, 3 no data
  int ice_class;
  int shaft_power; // horses
  int vhf;
  std::string lloyds_ship_type;
  int gross_tonnage;
  int laden_ballast;
  int heavy_oil, light_oil, diesel;
  int bunker_oil;  // tonnes
  int persons;
  int spare2;

  Ais8_1_24(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_1_24 const& msg);


// No message 8_1_15


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
  virtual Ais8_1_26_SensorEnum getType() const = 0;
  virtual ~Ais8_1_26_SensorReport()
  { };
  virtual void print()=0;
#if 1
  int report_type;
  int utc_day, utc_hr, utc_min;
  int site_id;  // aka link_id
#endif
};

// TODO: need pad?
Ais8_1_26_SensorReport* ais8_1_26_sensor_report_factory(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);

class Ais8_1_26_Location : public Ais8_1_26_SensorReport {
 public:
  float x, y, z;  // lon, lat, alt in m from MSL
  int owner, timeout;
  int spare;

  Ais8_1_26_Location(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_Location() {};
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_LOCATION;}
  void print();
};

class Ais8_1_26_Station : public Ais8_1_26_SensorReport {
 public:
  std::string name;
  int spare;

  Ais8_1_26_Station(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_Station() {/* */};
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_STATION;}
};

class Ais8_1_26_Wind : public Ais8_1_26_SensorReport {
 public:
  int wind_speed, wind_gust ; // knots
  int wind_dir, wind_gust_dir;
  int sensor_type;
  int wind_forcast, wind_gust_forcast; // knots
  int wind_dir_forcast;
  int utc_day_forcast, utc_hour_forcast, utc_min_forcast;
  int spare;

  Ais8_1_26_Wind(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_Wind() {};
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_WIND;}
};

class Ais8_1_26_WaterLevel : public Ais8_1_26_SensorReport {
 public:
  int type;
  float level; // m.  assuming it is being stored at 0.01 m inc.
  int trend;
  int vdatum;
  int sensor_type;
  int forcast_type;
  float level_forcast;
  int utc_day_forcast;
  int utc_hour_forcast;
  int utc_min_forcast;
  int duration; // minutes
  int spare;
  Ais8_1_26_WaterLevel(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_WaterLevel() {};
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_WATER_LEVEL;}
};

struct Ais8_1_26_Curr2D_Current {
  float speed; // knots
  int dir;
  int depth; // m
};

class Ais8_1_26_Curr2D : public Ais8_1_26_SensorReport {
 public:
  std::vector<Ais8_1_26_Curr2D_Current> currents;
  int type;
  int spare;

  Ais8_1_26_Curr2D(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_Curr2D() {};
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_CURR_2D;}
};

struct Ais8_1_26_Curr3D_Current {
  float north, east, up;
  int depth; // m
};

class Ais8_1_26_Curr3D : public Ais8_1_26_SensorReport {
 public:
  std::vector<Ais8_1_26_Curr3D_Current> currents;
  int type;
  int spare;

  Ais8_1_26_Curr3D(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_Curr3D() {};
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_CURR_3D;}
};

struct Ais8_1_26_HorzFlow_Current {
  int bearing, dist; // deg, m
  float speed; // knots
  int dir, level; // deg, m
};

class Ais8_1_26_HorzFlow : public Ais8_1_26_SensorReport {
 public:
  std::vector<Ais8_1_26_HorzFlow_Current> currents;
  int spare;

  Ais8_1_26_HorzFlow(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_HorzFlow() {};
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_HORZ_FLOW;}
};

class Ais8_1_26_SeaState : public Ais8_1_26_SensorReport {
 public:
  float swell_height;
  int swell_period, swell_dir; // s, deg
  int sea_state;
  int swell_sensor_type;
  float water_temp, water_temp_depth; // C, m
  int water_sensor_type;
  float wave_height;
  int wave_period, wave_dir; // s, deg
  int wave_sensor_type;
  float salinity;

  Ais8_1_26_SeaState(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_SeaState() {};
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_SEA_STATE;}
};

class Ais8_1_26_Salinity : public Ais8_1_26_SensorReport {
 public:
  float water_temp; // C
  float conductivity; // siemens/m
  float pressure; // decibars
  float salinity; // 0/00 ppt
  int salinity_type;
  int sensor_type;
  int spare;

  Ais8_1_26_Salinity(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_Salinity() {};
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_SALINITY;}
};

class Ais8_1_26_Wx : public Ais8_1_26_SensorReport {
 public:
  float air_temp; // C
  int air_temp_sensor_type;
  int precip;
  float horz_vis; // nm
  float dew_point; // C
  int dew_point_type;
  int air_pressure; // hPa
  int air_pressure_trend;
  int air_pressor_type;
  float salinity; // 0/00 ppt
  int spare;

  Ais8_1_26_Wx(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_Wx() {};
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_WX;}
};

class Ais8_1_26_AirDraught : public Ais8_1_26_SensorReport {
 public:
  float draught, gap, forcast_gap;
  int trend;
  int utc_day_forcast, utc_hour_forcast, utc_min_forcast;
  int spare;
  Ais8_1_26_AirDraught(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  Ais8_1_26_AirDraught() {};
  Ais8_1_26_SensorEnum getType() const {return AIS8_1_26_SENSOR_AIR_DRAUGHT;}
};

// IMO Circ 289 Environmental
class Ais8_1_26 : public Ais8 {
 public:
  std::vector<Ais8_1_26_SensorReport> reports;

  Ais8_1_26(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_1_26 const& msg);


// IMO Circ 289 Route information
class Ais8_1_27 : public Ais8 {
 public:
  int link_id;
  int sender_type, route_type;
  int utc_month, utc_day, utc_hour, utc_min;
  int duration;
  std::vector<AisPoint> waypoints;

  Ais8_1_27(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_1_27 const& msg);


//  No message 8_1_28


// IMO Circ 289 Text description
class Ais8_1_29 : public Ais8 {
 public:
  int link_id;
  std::string text;
  int spare2;

  Ais8_1_29(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_1_29 const& msg);


// No message 8_1_30

// IMO Circ 289 Meteorological and Hydrographic data
// Section 1, Table 1.1
// TODO: is this exactly the same as 8_1_11 or has anything changed?
//       x,y swapped.
class Ais8_1_31 : public Ais8 {
 public:
  float x, y;  // Opposite the bit order of 8_1_11
  int position_accuracy;  // New field
  int utc_day;
  int utc_hour;
  int utc_min;
  int wind_ave; // kts
  int wind_gust; // kts
  int wind_dir;
  int wind_gust_dir;
  float air_temp; // C
  int rel_humid;
  float dew_point;
  int air_pres;
  int air_pres_trend;
  float horz_vis; // NM
  float water_level; // m
  int water_level_trend;

  float surf_cur_speed;
  int surf_cur_dir;
  float cur_speed_2; // kts
  int cur_dir_2;
  int cur_depth_2; // m
  float cur_speed_3; // kts
  int cur_dir_3;
  int cur_depth_3; // m

  float wave_height; // m
  int wave_period;
  int wave_dir;
  float swell_height; // m
  int swell_period;
  int swell_dir;
  int sea_state; // beaufort scale - Table 1.2
  float water_temp;
  int precip_type;
  float salinity;
  int ice; // yes/no/undef/unknown
  int spare2;

  Ais8_1_31(const char *nmea_payload, const size_t pad);
  void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_1_31 const& msg);


// ECE-TRANS-SC3-2006-10e-RIS.pdf - River Information System
// Inland ship static and voyage related data
class Ais8_200_10 : public Ais8 {
 public:
  std::string eu_id;  // European Vessel ID - 8 characters
  float length, beam; // m
  int ship_type;
  int haz_cargo;
  float draught;
  int loaded;
  int speed_qual, course_qual, heading_qual; // sensor quality
  int spare2;
  Ais8_200_10(const char *nmea_payload, const size_t pad);
  // void print();
};
// std::ostream& operator<< (std::ostream& o, Ais8_200_10 const& msg);

  // 21 and 22 do not exist

// ECE-TRANS-SC3-2006-10e-RIS.pdf - River Information System
class Ais8_200_23 : public Ais8 {
 public:
  int utc_year_start, utc_month_start, utc_day_start;
  int utc_year_end, utc_month_end, utc_day_end;
  int utc_hour_start, utc_min_start;
  int utc_hour_end, utc_min_end;
  float x1,y1;
  float x2,y2;
  int type;
  int min;
  int max;
  int classification;
  int wind_dir; // EMMA CODE
  int spare2;

  Ais8_200_23(const char *nmea_payload, const size_t pad);
  // void print();
};
// std::ostream& operator<< (std::ostream& o, Ais8_200_23 const& msg);

// ECE-TRANS-SC3-2006-10e-RIS.pdf - River Information System
// Water Level
class Ais8_200_24 : public Ais8 {
 public:
  std::string country;
  int guage_ids[4];
  float levels[4]; // m
  Ais8_200_24(const char *nmea_payload, const size_t pad);
  // void print();
};
// std::ostream& operator<< (std::ostream& o, Ais8_200_24 const& msg);

// ECE-TRANS-SC3-2006-10e-RIS.pdf - River Information System
class Ais8_200_40 : public Ais8 {
 public:
  float x,y;
  int form;
  int dir; // degrees
  int stream_dir;
  int status_raw;
  //int status[9];  // WTF is the encoding for this?
  int spare2;
  Ais8_200_40(const char *nmea_payload, const size_t pad);
  // void print();
};
// std::ostream& operator<< (std::ostream& o, Ais8_200_40 const& msg);

// ECE-TRANS-SC3-2006-10e-RIS.pdf - River Information System
class Ais8_200_55 : public Ais8 {
 public:
  int crew;
  int passengers;
  int yet_more_personnel; // WTF?  Like a maid or waiter?
  int spare2[3]; // JERKS... why 51 spare bits?
  Ais8_200_55(const char *nmea_payload, const size_t pad);
  // void print();
};
// std::ostream& operator<< (std::ostream& o, Ais8_200_55 const& msg);



// New IMO Circ 289 Area notice broadcast is DAC 1, FI 22
// US will use the RTCM Regional Message.  DAC 366, FI 22
// Hopefully, the two shall become the same.  -kds 10/2010
#include "ais8_366_22.h"


// Old Zone message for SBNMS / Boston right whales
#if 0
class Ais8_366_34 : public Ais8 {
public:
    int zone_id;
    int zone_type;
    int day;
    int hour;
    int minute;
    int dur_min; // duration in minutes

    Ais8_366_34(const char *nmea_payload, const size_t pad=0);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_366_34 const& msg);
#endif

class Ais9 : public AisMsg {
public:
    int alt; // m above sea level
    float sog;
    int position_accuracy;
    float x, y; // Long and lat
    float cog;
    int timestamp;
    int alt_sensor;
    int spare;
    int dte; // FIX: bool?
    int spare2;
    int assigned_mode;
    bool raim;
    int commstate_flag;

    int sync_state; // In both SOTDMA and ITDMA

    // SOTDMA
    int slot_timeout;
    bool slot_timeout_valid;

    // Based on slot_timeout which ones are valid
    int received_stations;
    bool received_stations_valid;

    int slot_number;
    bool slot_number_valid;

    bool utc_valid;  // Only means that the values are set.  Can still have
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

    Ais9(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais9 const& msg);

// 10 ":" - UTC and date inquiry
class Ais10 : public AisMsg {
public:
    int spare;
    int dest_mmsi;
    int spare2;

    Ais10(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais10 const& msg);

// 11 ';' - See 4_11

// '<' - Addressd safety related
class Ais12 : public AisMsg {
public:
    int seq_num;
    int dest_mmsi;
    bool retransmitted;
    int spare;
    std::string text;

    Ais12(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais12 const& msg);

// 13 '=' - See 7

// '>' - Safety broadcast
class Ais14 : public AisMsg {
public:
    int spare;
    std::string text;
    int expected_num_spare_bits; // The bits in the nmea_payload not used
    Ais14(const char *nmea_payload);
    void print();
};

std::ostream& operator<< (std::ostream& o, Ais14 const& msg);


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

    Ais15(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais15 const& msg);

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

    Ais16(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais16 const& msg);

// ITU-R M.823  http://www.itu.int/rec/R-REC-M.823/en
struct GnssCorrection17 {
    int msg_type;
    int station_id;
    int z_cnt;
    //int n;
    int health;
  // TODO: DGNSS data word - what is their word size?
};

// A - GNSS broacast - FIX: only partially coded
class Ais17 : public AisMsg {
public:
    int spare;
    float x, y;
    int spare2;

  // TODO: These should be lists for each payload?  Use a struct?
#if 0
    int msg_type;
    int station_id;
    int z_cnt;
    int n;
    int health;
#endif
  // std::vector<GnssCorrection17> corrections;

    // Anyfield below here is subject to change
  //std::vector<unsigned char> payload; // Up to 29 words... FIX: what should be here?

    Ais17(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais17 const& msg);


// B - Class B position report
class Ais18 : public AisMsg {
public:
    int spare;
    float sog;
    int position_accuracy;
    float x, y; // Long and lat
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

    bool utc_valid;  // Only means that the values are set.  Can still have
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

    Ais18(const char *nmea_payload);
    void print();
};

std::ostream& operator<< (std::ostream& o, Ais18 const& msg);

// C - Class B extended ship and position
class Ais19 : public AisMsg {
public:
    int spare;
    float sog;
    int position_accuracy;
    float x, y; // Long and lat
    float cog;
    int true_heading;
    int timestamp;
    int spare2;
    std::string name;
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

    Ais19(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais19 const& msg);

// 'D' - Data link management - FIX: not yet coded
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

    Ais20(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais20 const& msg);

// 'E' - Aids to navigation report - FIX: not yet coded
class Ais21 : public AisMsg {
public:

    int aton_type;
    std::string name;
    int position_accuracy;
    float x,y;
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

    Ais21(const char *nmea_payload, const size_t pad=0);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais21 const& msg);

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
    float x1,y1;
    float x2,y2;

    // if addressed is true
    bool dest_valid;
    int dest_mmsi_1;
    int dest_mmsi_2;

    int chan_a_bandwidth;
    int chan_b_bandwidth;
    int zone_size;

    int spare2; // Lame that they make a huge spare here.  Bad bad bad

    Ais22(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais22 const& msg);

// 'G' - Group Assignment Command - FIX: not yet coded
class Ais23 : public AisMsg {
public:
    int spare;
    float x1,y1;
    float x2,y2;
    int station_type;
    int type_and_cargo;

    int spare2; // 22 bits of spare here?  what were people thinking?

    int txrx_mode;
    int interval_raw; // raw value, not sec
    int quiet;
    int spare3;

    Ais23(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais23 const& msg);


// Class B Static Data report
class Ais24 : public AisMsg {
public:
    int part_num;

    // Part A
    std::string name;

    // Part B
    int type_and_cargo;
    std::string vendor_id;
    std::string callsign;
    int dim_a;
    int dim_b;
    int dim_c;
    int dim_d;
    int spare;

    Ais24(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais24 const& msg);

// 'I' - Single slot binary message - addressed or broadcast - TODO: handle payload
class Ais25 : public AisMsg {
public:
  //bool addressed; // broadcast if false - destination indicator
    bool use_app_id; // if false, payload is unstructured binary.  Commentary: do not use with this false

    bool dest_mmsi_valid;
    int dest_mmsi; // only valid if addressed
    //std::vector<unsigned char> payload; // If unstructured.  Yuck.

    int dac; // valid it use_app_id
    int fi;

    Ais25(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais25 const& msg);

// 'J' - Multi slot binary message with comm state - TODO: handle payload
class Ais26 : public AisMsg {
public:
  //bool addressed; // broadcast if false - destination indicator
    bool use_app_id; // if false, payload is unstructured binary.  Commentary: do not use with this false

    bool dest_mmsi_valid;
    int dest_mmsi; // only valid if addressed

    int dac; // valid it use_app_id
    int fi;

    //std::vector<unsigned char> payload; // If unstructured.  Yuck.

    int commstate_flag; // 0 - SOTDMA, 1 - TDMA

    // SOTDMA
    int sync_state;
    int slot_timeout;
    bool slot_timeout_valid;

    // Based on slot_timeout which ones are valid
    int received_stations;
    bool received_stations_valid;

    int slot_number;
    bool slot_number_valid;

    bool utc_valid;  // Only means that the values are set.  Can still have
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

    Ais26(const char *nmea_payload, const size_t pad=0);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais26 const& msg);

// K - Long-range position report - e.g. for satellite receivers
class Ais27 : public AisMsg {
public:
    int position_accuracy;
    bool raim ;
    int nav_status;
    float x, y;
    int sog;
    int cog;
    bool gnss;  // warning: bits in AIS are flipped sense
    int spare;

    Ais27(const char *nmea_payload, const size_t pad=0);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais27 const& a);

//////////////////////////////////////////////////////////////////////
// Support templates for decoding
//////////////////////////////////////////////////////////////////////

extern std::bitset<6> nmea_ord[128];

template<size_t T>
AIS_STATUS aivdm_to_bits(std::bitset<T> &bits, const char *nmea_payload) {
    assert(nmea_payload);
    //assert(nmea_ord_initialized);
    if (strlen(nmea_payload) > T/6) {
#ifndef NDEBUG
        std::cerr << "ERROR: message longer than max allowed size (" << T/6 << "): found "
                  << strlen(nmea_payload) << " characters in "
                  << nmea_payload << std::endl;
#endif
        return AIS_ERR_MSG_TOO_LONG;
    }
    //assert (strlen(nmea_payload) <= T/6 );
    for (size_t char_idx=0; nmea_payload[char_idx] != '\0' && char_idx < T/6; char_idx++) {
        int c = int(nmea_payload[char_idx]);
        if (c<48 or c>119 or (c>=88 and c<=95) ) {
            //std::cout << "bad character: '" << nmea_payload[char_idx] << "' " << c << std::endl;
            return AIS_ERR_BAD_NMEA_CHR;
        }
        const std::bitset<6> bs_for_char = nmea_ord[ c ];
        for (size_t offset=0; offset < 6 ; offset++) {
            bits[char_idx*6 + offset] = bs_for_char[offset];
        }
    }
    return AIS_OK;
}

template<size_t T>
int ubits(const std::bitset<T> &bits, const size_t start, const size_t len)
{
    assert (len <= 32);
    assert (start+len <= T);
    std::bitset<32> bs_tmp;
    for (size_t i = 0; i < len; i++)
        bs_tmp[i] = bits[start+len-i-1];
    return bs_tmp.to_ulong();
}

typedef union {
    long long_val;
    unsigned long ulong_val;
} long_union;

template<size_t T>
int sbits(std::bitset<T> bs, const size_t start, const size_t len) {
    assert (len <= 32);
    assert (start+len <= T); // FIX:  should it just be < ?
    std::bitset<32> bs32;
    if (len < 32 && 1 == bs[start] ) bs32.flip(); // pad 1's to the left if negative

    for (size_t i = 0; i < len; i++)
        bs32[i] = bs[start+len-i-1];

    long_union val;
    val.ulong_val = bs32.to_ulong();
    return val.long_val;
}

extern const std::string bits_to_char_tbl;

template<size_t T>
const std::string ais_str(const std::bitset<T> &bits, const size_t start, const size_t len) {
    assert (start+len < T);
    assert (len % 6 == 0);
    const size_t num_char = len / 6;
    std::string result(num_char, '@');
    //cout << "str: " << T << " " << start << " " << len << " " << num_char << " " << result << endl;
    for (size_t char_idx=0; char_idx < num_char; char_idx++) {
        const int char_num = ubits(bits, start+char_idx*6, 6);
        result[char_idx] = bits_to_char_tbl[char_num];
    }
    return result;
}

#endif
