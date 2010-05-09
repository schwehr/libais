// Since Apr 2010
// g++ ais_pos.cxx -o ais_pos -g -Wall -O3 -Wimplicit -W -Wredundant-decls -pedantic  -funroll-loops -fexpensive-optimizations 

#include "ais.h"

#include <iostream>
#include <bitset>
#include <string>
#include <cassert>
#include <cmath>
//using namespace std;

std::bitset<6> nmea_ord[128];
bool nmea_ord_initialized = false;

void build_nmea_lookup() {
    //std::cout << "building lut..." << std::endl;
    for (int c=0; c < 128; c++) {
        int val = c - 48;
        if (val>=40) val-= 8;
        if (val < 0) continue;
        std::bitset<6> bits(val);
        bool tmp;
        tmp = bits[5]; bits[5] = bits[0]; bits[0] = tmp;
        tmp = bits[4]; bits[4] = bits[1]; bits[1] = tmp;
        tmp = bits[3]; bits[3] = bits[2]; bits[2] = tmp;
        nmea_ord[c] = bits;
    }
    nmea_ord_initialized = true;
}

// for decoding str bits inside of a binary message
const std::string bits_to_char_tbl="@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^- !\"#$%&`()*+,-./0123456789:;<=>?";

const char * const AIS_STATUS_STRINGS[AIS_STATUS_NUM_CODES] = {
    "AIS_ERR_NONE",
    "AIS_ERR_BAD_BIT_COUNT",
    "AIS_ERR_WRONG_MSG_TYPE",
    "AIS_ERR_BAD_NMEA_CHR",
    "AIS_ERR_BAD_PTR",
    "AIS_ERR_UNKNOWN_MSG_TYPE",
    "AIS_ERR_MSG_NOT_IMPLEMENTED",
    "AIS_ERR_BAD_MSG_CONTENT",
    "AIS_ERR_EXPECTED_STRING"
};
