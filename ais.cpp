// Since Apr 2010
// g++ ais_pos.cxx -o ais_pos -g -Wall -O3 -Wimplicit -W -Wredundant-decls -pedantic  -funroll-loops -fexpensive-optimizations 

#include "ais.h"

#include <iostream>
#include <bitset>
#include <string>
#include <cassert>
#include <cmath>
//using namespace std;


// for decoding str bits inside of a binary message
const std::string bits_to_char_tbl="@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^- !\"#$%&`()*+,-./0123456789:;<=>?";

const char * const AIS_STATUS_STRINGS[AIS_STATUS_NUM_CODES] = {
    "AIS_OK",
    "AIS_ERR_BAD_BIT_COUNT",
    "AIS_ERR_WRONG_MSG_TYPE",
    "AIS_ERR_BAD_NMEA_CHR",
    "AIS_ERR_BAD_PTR",
    "AIS_ERR_UNKNOWN_MSG_TYPE",
    "AIS_ERR_MSG_NOT_IMPLEMENTED",
    "AIS_ERR_BAD_MSG_CONTENT",
    "AIS_ERR_EXPECTED_STRING",
    "AIS_ERR_MSG_TOO_LONG",
};

#ifndef NDEBUG
static void check_error_messages() {
    
    std::cout << "AIS_OK:                       " << AIS_OK << " " << AIS_STATUS_STRINGS[AIS_OK] << std::endl;
    std::cout << "AIS_ERR_BAD_BIT_COUNT:        " << AIS_ERR_BAD_BIT_COUNT << " " << AIS_STATUS_STRINGS[AIS_ERR_BAD_BIT_COUNT] << std::endl;
    std::cout << "AIS_ERR_WRONG_MSG_TYPE:       " << AIS_ERR_WRONG_MSG_TYPE << " " << AIS_STATUS_STRINGS[AIS_ERR_WRONG_MSG_TYPE] << std::endl;
    std::cout << "AIS_ERR_BAD_NMEA_CHR:         " << AIS_ERR_BAD_NMEA_CHR << " " << AIS_STATUS_STRINGS[AIS_ERR_BAD_NMEA_CHR] << std::endl;
    std::cout << "AIS_ERR_BAD_PTR:              " << AIS_ERR_BAD_PTR << " " << AIS_STATUS_STRINGS[AIS_ERR_BAD_PTR] << std::endl;
    std::cout << "AIS_ERR_UNKNOWN_MSG_TYPE:     " << AIS_ERR_UNKNOWN_MSG_TYPE << " " << AIS_STATUS_STRINGS[AIS_ERR_UNKNOWN_MSG_TYPE] << std::endl;
    std::cout << "AIS_ERR_MSG_NOT_IMPLEMENTED:  " << AIS_ERR_MSG_NOT_IMPLEMENTED << " " << AIS_STATUS_STRINGS[AIS_ERR_MSG_NOT_IMPLEMENTED] << std::endl;
    std::cout << "AIS_ERR_BAD_MSG_CONTENT:      " << AIS_ERR_BAD_MSG_CONTENT << " " << AIS_STATUS_STRINGS[AIS_ERR_BAD_MSG_CONTENT] << std::endl;
    std::cout << "AIS_ERR_EXPECTED_STRING:      " << AIS_ERR_EXPECTED_STRING << " " << AIS_STATUS_STRINGS[AIS_ERR_EXPECTED_STRING] << std::endl;
    std::cout << "AIS_ERR_MSG_TOO_LONG:         " << AIS_ERR_MSG_TOO_LONG << " " << AIS_STATUS_STRINGS[AIS_ERR_MSG_TOO_LONG] << std::endl;

}
#endif

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

#ifndef NDEBUG
    std::cout << "checking error messages..." << std::endl;
    check_error_messages();
#endif

}
