#include "ais.h"

#include <iostream>
#include <bitset>
//#include <string>
#include <cassert>
//#include <vector>
using namespace std;

Ais7_13::Ais7_13(const char *nmea_payload) {
    assert (nmea_payload);
    const size_t num_bits = strlen(nmea_payload) * 6;
    cout << "num_bits: " << num_bits << endl;
    
    assert ( (40+32*1)==num_bits or (40+32*2)==num_bits or (40+32*3)==num_bits or (40+32*4) == num_bits);

    std::bitset<168> bs;
    CHECKPOINT;

    aivdm_to_bits(bs, nmea_payload);

    message_id = ubits(bs, 0, 6);
    cout << "message_id: " << message_id << endl;
    assert (message_id == 7 or message_id == 13);
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    spare = ubits(bs,38,2);

    const size_t num_acks = (num_bits - 40) / 32;
    for (size_t i=0; i < num_acks; i++) {
        const size_t start = 40 + i*32;
        cout << i << ": " << start << endl;
        const int id = ubits(bs,40+i*32,30);
        cout << "id: " << id << endl;
        const int seq = ubits(bs,40+i*32+30,2);
        dest_mmsi.push_back(id);
        seq_num.push_back(seq);
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
