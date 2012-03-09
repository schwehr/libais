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
#define LIBAIS_VERSION_MINOR 7

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

// FIX: figure out how to handle Ais6

// msg 6 and 12 ack 
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
    //protected:
public:
    Ais8() {}

    static const int MAX_BITS = AIS8_MAX_BITS; //1192; //1008;

    int spare;
    int dac; // dac+fi = app id
    int fi;
    
    std::vector<unsigned char> payload; // If dac/fi (app id is now one we know).  without dac/fi
    
    Ais8(const char *nmea_payload);
    bool decode_header8(const std::bitset<MAX_BITS> &bs);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais8 const& msg);

// IMO met hydro
class Ais8_1_11 : public Ais8 {
public:
#if 0
    int message_id;
    int repeat_indicator;
    int mmsi; // source ID
    int spare;
    int dac; // dac+fi = app id
    int fi;
#endif
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
    float wave_height; // ,
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
    int extended_water_level; //spare;  // OHMEX uses this for extra water level precision

    Ais8_1_11(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_1_11 const& msg);


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
    

    Ais8_366_34(const char *nmea_payload, const int pad=0);
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

    // SOTDMA
    //int sync_state;
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

// 10 - ":" UTC and date inquiry
class Ais10 : public AisMsg {
public:
    int spare;
    int dest_mmsi;
    int spare2;

    Ais10(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais10 const& msg);

// 11 - See 4_11

// < - Addressd safety related
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

// Safety broadcast
class Ais14 : public AisMsg {
public:
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

// @ - Assigned mode command - FIX: not yet coded
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

// A? - GNSS broacast - FIX: not yet coded
class Ais17 : public AisMsg {
public:
    int spare;
    float x, y;
    int spare2;

    int msg_type;
    int station_id;
    int z_cnt;
    int n;
    int health;

    std::vector<unsigned char> payload; // Up to 29 words... FIX: what should be here?

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

    Ais21(const char *nmea_payload, const int pad=0);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais21 const& msg);

// 'F' - Channel Management - FIX: not yet coded
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
    // int interval_sec;

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

// 'I' - Single slot binary message - addressed or broadcast - FIX: not yet coded
class Ais25 : public AisMsg {
public:
    bool addressed; // broadcast if false - destination indicator
    bool use_app_id; // if false, payload is unstructured binary.  Commentary: do not use with this false

    int dest_mmsi; // only valid if addressed
    std::vector<unsigned char> payload; // If unstructured.  Yuck.

    Ais25(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais25 const& msg);

// 'J' - Multi slot binary message with comm state - FIX: not yet coded
class Ais26 : public AisMsg {
public:
    bool addressed; // broadcast if false - destination indicator
    bool use_app_id; // if false, payload is unstructured binary.  Commentary: do not use with this false
  
    std::vector<unsigned char> payload; // If unstructured.  Yuck.

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
 
    Ais26(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais26 const& msg);


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
