// Since Apr 2010
// g++ ais_pos.cxx -o ais_pos -g -Wall -O3 -Wimplicit -W -Wredundant-decls -pedantic  -funroll-loops -fexpensive-optimizations 

#include "ais.h"

#include <iostream>
#include <bitset>
#include <string>
#include <cassert>
#include <cmath>
#include "ais8_200_10.h"
//using namespace std;

AisMsg::~AisMsg()
{
}

const std::string nth_field(const std::string &str, const size_t n, const char c) {
    // FIX: handle the off the end case better
    size_t pos;
    size_t count;
    for (pos=0, count=0; count < n and pos != std::string::npos; count+=1) {
        if (pos>0) pos += 1; // Skip past the current char that matched
        pos = str.find(c, pos);
    } 
    if (std::string::npos == pos) return std::string("");
   
    const size_t start = pos;
    const size_t end = str.find(c, pos+1);
    if (std::string::npos == end) return str.substr(start);
    return str.substr(start+1, end-start-1);
}

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


std::bitset<6> nmea_ord[128];
bool nmea_ord_initialized = false;

//#ifndef NDEBUG
#if 0
#definte VERIFY_ERROR_MESSAGES
#endif

#ifdef VERIFY_ERROR_MESSAGES
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

#ifdef VERIFY_ERROR_MESSAGES
    std::cout << "checking error messages..." << std::endl;
    check_error_messages();
#endif

}


// decode function "ported" from python bindings
AisMsg *decode(std::string const &nmea_payload)
{
    if (!nmea_ord_initialized) {
        build_nmea_lookup();
    }

    AisMsg *result=NULL;
    switch (nmea_payload[0]) {
        
        // Class A Position
        case '1': // FALLTHROUGH
        case '2': // FALLTHROUGH
        case '3':
            result = new Ais1_2_3(nmea_payload.c_str());
            break;
            
            // 4 - Basestation report
            // 11 - UTC date response
        case '4': // FALLTHROUGH 
        case ';':
            result = new Ais4_11(nmea_payload.c_str());
            break;
            
        case '5': // 5 - Ship and Cargo
        result = new Ais5(nmea_payload.c_str());
        break;
        
        case '6': // 6 - Addressed binary message
        // result = ais6_to_pydict(nmea_payload);
        // returning NULL in C++ version - PyErr_Format(ais_py_exception, "ais.decode: message 6 not yet handled");
        break;
        
        // 7 - ACK for addressed binary message
        // 13 - ASRM Ack  (safety message)
        case '=': // FALLTHROUGH
        case '7':
            //std::cerr << "7_or_14: " << nmea_payload << std::endl;
            result = new Ais7_13(nmea_payload.c_str());
            break;
            
        case '8': // 8 - Binary broadcast message (BBM)
        {
            Ais8* ais8 = new Ais8(nmea_payload.c_str());
            result = ais8;
            switch(ais8->dac)
            {
                case 1:
                    switch(ais8->fi)
                    {
                        case 22:
                        {
                            result = new Ais8_001_22(nmea_payload.c_str());
                            break;
                        }
                    }
                    break;
                case 200:
                    switch(ais8->fi)
                    {
                        case 10:
                        {
                            result = new Ais8_200_10(nmea_payload.c_str());
                        }
                    }
                case 366:
                    switch(ais8->fi)
                    {
                        case 22:
                        {
                            result = new Ais8_366_22(nmea_payload.c_str());
                            break;
                        }
                    }
                    break;
            }
        }
        break;
        
        case '9': // 9 - SAR Position
        result = new Ais9(nmea_payload.c_str());
        // result = ais9_to_pydict(nmea_payload);
        //PyErr_Format(ais_py_exception, "ais.decode: message 9 not yet handled");
        break;
        
        case ':': // 10 - UTC Query
        result = new Ais10(nmea_payload.c_str());
        //result = ais10_to_pydict(nmea_payload);
        //PyErr_Format(ais_py_exception, "ais.decode: message 10 (;) not yet handled");
        break;
        
        // ':' 11 - See 4
        
        case '<': // 12 - ASRM
        result = new Ais12(nmea_payload.c_str());
        // result = ais12_to_pydict(nmea_payload);
        //PyErr_Format(ais_py_exception, "ais.decode: message 12 (<) not yet handled");
        break;
        
        // 13 - See 7
        
        case '>': // 14 - SRBM - Safety broadcast
        result = new Ais14(nmea_payload.c_str());
        break;
        
        case '?': // 15 - Interrogation
        result = new Ais15(nmea_payload.c_str());
        // result = ais15_to_pydict(nmea_payload);
        //PyErr_Format(ais_py_exception, "ais.decode: message 15 (?) not yet handled");
        break;
        
        case '@': // 16 - Assigned mode command
        // result = ais16_to_pydict(nmea_payload);
        //PyErr_Format(ais_py_exception, "ais.decode: message 16 (@) not yet handled");
        break;
        
        case 'A': // 17 - GNSS broadcast
        // result = ais17_to_pydict(nmea_payload);
        //PyErr_Format(ais_py_exception, "ais.decode: message 17 (A) not yet handled");
        break;
        
        case 'B': // 18 - Position, Class B
        result = new Ais18(nmea_payload.c_str());
        break;
        
        case 'C': // 19 - Position and ship, Class B
        result = new Ais19(nmea_payload.c_str());
        break;
        
        case 'D': // 20 - Data link management
        // result = ais20_to_pydict(nmea_payload);
        //PyErr_Format(ais_py_exception, "ais.decode: message 20 (D) not yet handled");
        break;
        
        case 'E': // 21 - Aids to navigation report
        result = new Ais21(nmea_payload.c_str());
        //PyErr_Format(ais_py_exception, "ais.decode: message 21 (E) not yet handled");
        break;
        
        case 'F': // 22 - Channel Management
        // result = ais22_to_pydict(nmea_payload);
        //PyErr_Format(ais_py_exception, "ais.decode: message 22 (F) not yet handled");
        break;
        
        case 'G': // 23 - Group Assignment Command
        // result = ais23_to_pydict(nmea_payload);
        //PyErr_Format(ais_py_exception, "ais.decode: message 23 (G) not yet handled");
        break;
        
        case 'H': // 24 - Static data report
        result = new Ais24(nmea_payload.c_str());
        break;
        
        case 'I': // 25 - Single slot binary message - addressed or broadcast
        // result = ais25_to_pydict(nmea_payload);
        //PyErr_Format(ais_py_exception, "ais.decode: message 25 (I) not yet handled");
        break;
        
        case 'J': // 26 - Multi slot binary message with comm state
        // result = ais26_to_pydict(nmea_payload);
        //PyErr_Format(ais_py_exception, "ais.decode: message 26 (J) not yet handled");
        break;
        
        // 27 - K
        // 28 - L
        
        default:
            result = NULL;
            //assert (false);
            //std::cout << "Unknown message type: '" << nmea_payload[0] << "'\n"
            //          << "\tline: " << nmea_payload << std::endl;
            //PyErr_Format(ais_py_exception, "ais.decode: message %c not known", nmea_payload[0]);
            
    }
    
    return result;
}