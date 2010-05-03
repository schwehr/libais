// Since Apr 2010
// g++ ais_pos.cxx -o ais_pos -g -Wall -O3 -Wimplicit -W -Wredundant-decls -pedantic  -funroll-loops -fexpensive-optimizations 

#include <bitset>
using namespace std;

void build_nmea_lookup();
 
void aivdm_to_bits(bitset<168> &bits, const char *nmea_payload);

class AisPos123 {
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

    AisPos123(const char *nmea_payload);
};

