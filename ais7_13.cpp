#include "ais.h"

#include <iostream>
#include <bitset>
//#include <string>
#include <cassert>
//#include <vector>
//using namespace std;

Ais7_13::Ais7_13(const char *nmea_payload) {
    init();

    assert (nmea_payload);
    const size_t num_bits = strlen(nmea_payload) * 6;
    //cout << "num_bits: " << num_bits << endl;
    
    if (! ((40+32*1)==num_bits or (40+32*2)==num_bits or (40+32*3)==num_bits or (40+32*4)==num_bits) ){
        status = AIS_ERR_BAD_BIT_COUNT;
        return;
    }       

    std::bitset<168> bs;
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (message_id != 7 and message_id != 13) {
        status = AIS_ERR_WRONG_MSG_TYPE; 
        return;
    }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);
    spare = ubits(bs,38,2);

    const size_t num_acks = (num_bits - 40) / 32;
    for (size_t i=0; i < num_acks; i++) {
        dest_mmsi.push_back(ubits(bs,40+i*32,30));
        seq_num.push_back(ubits(bs,40+i*32+30,2));
    }
}

void
Ais7_13::print() {
    std::cout << message_id << ( (7==message_id)?" ABM Ack":" safety ack" ) << "\n"
              << "\tmmsi: " << mmsi << " repeat: " << repeat_indicator << "\n";
    std::cout << "\tspare: " << spare << "\n";
    for (size_t i=0; i < dest_mmsi.size(); i++) {
        std::cout << "\tack: " << dest_mmsi[i] << " seq: " << seq_num[i] << "\n";
    }

    std::cout << std::endl;
}
