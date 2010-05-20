// Since 2010-05-19
// Binary Broadcast Message (BBM) - 8

#include "ais.h"

Ais8::Ais8(const char *nmea_payload) {
    assert(nmea_payload);
    init();
    const int payload_len = strlen(nmea_payload)*6 - 46; // in bits w/o DAC/FI
    std::cout << "payload_len: " << strlen(nmea_payload) << " " << strlen(nmea_payload)*6 << " " << payload_len << " " << payload_len / 8 << "\n";
    if (payload_len < 0 or payload_len > 952) {
        status = AIS_ERR_BAD_BIT_COUNT;
        return;
    }

    std::bitset<MAX_BITS> bs;  // FIX: shouldn't this be a max of 1192?
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;  // checks status

    if (!decode_header8(bs)) return; // side effect - sets status

    // Handle all the byte aligned payload
    for (int i=0; i<payload_len/8; i++) {
        const int start = 56+i*8;
        //std::cout << "payload: " << i << " " << start <<"\n";
        payload.push_back(ubits(bs,start,8));
    }
    const int remainder = payload_len % 8; // FIX: need to handle spare bits!!
    std::cout << "remainder: " << remainder << "\n";
    if (remainder > 0) {
        const int start = (payload_len/8) * 8;
        //std::cout << "start: " << start <<"\n";
        payload.push_back(ubits(bs, start, remainder));
    }
}

bool Ais8::decode_header8(const std::bitset<MAX_BITS> &bs) {
    message_id = ubits(bs, 0, 6);
    if (8 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return false; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);
    spare = ubits(bs,38,2);
    dac = ubits(bs,40,10);
    fi = ubits(bs,50,6);
    return true;
}

#include <iomanip>

void Ais8::print() {
    std::cout << "AIS_broadcast_binary_message: " << message_id 
              << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
    std::cout << "\tpayload: "; // << std::hex << std::uppercase; // << std::setfill('0') << std::setw(2) << "\n";
    for (std::vector<unsigned char>::iterator i = payload.begin(); i != payload.end(); i++) {
        std::cout << std::hex <<std::setfill('0') << std::setw(2)<< int(*i);
    }
    std::cout << std::dec << std::nouppercase << std::endl;
    //std::cout << "test: " << 1 << " " << 255 << " " << std::hex << 255 << std::endl;
}
