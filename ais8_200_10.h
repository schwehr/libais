#ifndef AIS8_200_10_H
#define AIS8_200_10_H

#include "ais.h"

/// Inland Waterways ship static and voyage BBM payload.
/// http://www.e-navigation.nl/node/28

class Ais8_200_10 : public Ais8 {
public:
    std::string vessel_id_eu;
    float length;
    float beam;
    int ship_type;
    int hazardous_cargo;
    float draught;
    int loaded;
    int speed_quality;
    int course_quality;
    int heading_quality;
    
    Ais8_200_10(const char *nmea_payload);
    ~Ais8_200_10();
    void print();
    
};

#endif
