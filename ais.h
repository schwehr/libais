// -*- c++ -*-
// Since Apr 2010
// g++ ais_pos.cxx -o ais_pos -g -Wall -O3 -Wimplicit -W -Wredundant-decls -pedantic  -funroll-loops -fexpensive-optimizations 

#include <bitset>
#include <string>
#include <cassert>
#include <vector>
#include <cstring>

#include <iostream> // for checkpoint

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
    AIS_STATUS_NUM_CODES
};

extern const char *const AIS_STATUS_STRINGS[AIS_STATUS_NUM_CODES];

class AisMsg {
public:
    bool had_error() {return status != AIS_OK;}
    AIS_STATUS get_error() {return status;}
protected:
    AIS_STATUS status; // AIS_OK or error code
    void init() {status = AIS_OK;}
};

class Ais1_2_3 : public AisMsg {
public:
    int message_id;
    int repeat_indicator;
    int mmsi;
    int nav_status;
    bool rot_over_range;
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

    bool keep_flag;
    bool keep_flag_valid;

    Ais1_2_3(const char *nmea_payload);
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais1_2_3 const& a);

// 4 bsreport and 11 utc date response
class Ais4_11 : public AisMsg {
 public:
    int message_id;
    int repeat_indicator;
    int mmsi;
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
    int message_id;
    int repeat_indicator;
    int mmsi;
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
    int message_id;
    int repeat_indicator;
    int mmsi; // source ID

    int spare;

    std::vector<int> dest_mmsi;
    std::vector<int> seq_num;

    Ais7_13(const char *nmea_payload);
    void print();
};

std::ostream& operator<< (std::ostream& o, Ais7_13 const& msg);

// Safety broadcast
class Ais14 : public AisMsg {
public:
    int message_id;
    int repeat_indicator;
    int mmsi;
    std::string text;
    int expected_num_spare_bits; // The bits in the nmea_payload not used
    Ais14(const char *nmea_payload);
    void print();
};

std::ostream& operator<< (std::ostream& o, Ais14 const& msg);

// B - Class B position report
class Ais18 : public AisMsg {
public:
    int message_id;
    int repeat_indicator;
    int mmsi;
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
    int message_id;
    int repeat_indicator;
    int mmsi;
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


#define CHECKPOINT std::cerr <<  __FILE__ << ":" << __LINE__ << " checkpoint" << std::endl

//////////////////////////////////////////////////////////////////////
// Support templates for decoding
//////////////////////////////////////////////////////////////////////

extern std::bitset<6> nmea_ord[128];

template<size_t T>
AIS_STATUS aivdm_to_bits(std::bitset<T> &bits, const char *nmea_payload) {
    assert (nmea_payload);
    assert (strlen(nmea_payload) <= T/6 );
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
    
    const size_t num_char = len / 6;
    std::string result(num_char, '@');
    //cout << "str: " << T << " " << start << " " << len << " " << num_char << " " << result << endl;
    for (size_t char_idx=0; char_idx < num_char; char_idx++) {
        const int char_num = ubits(bits, start+char_idx*6, 6);
        result[char_idx] = bits_to_char_tbl[char_num];
    }
    return result;
}

