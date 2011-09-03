// Since 2010-05-19
// Binary Broadcast Message (BBM) - 8

#include "ais.h"

Ais8::Ais8(const char *nmea_payload) {
    assert(nmea_payload);
	assert(nmea_ord_initialized); // Make sure we have the lookup table built
    init();
    const int payload_len = strlen(nmea_payload)*6 - 46; // in bits w/o DAC/FI
    //std::cout << "payload_len: " << strlen(nmea_payload) << " " << strlen(nmea_payload)*6 << " " << payload_len << " " << payload_len / 8 << "\n";
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
    //std::cout << "remainder: " << remainder << "\n";
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


//////////////////////////////////////////////////////////////////////
Ais8_1_11::Ais8_1_11(const char *nmea_payload) {
    assert(nmea_payload);
    init();
    //CHECKPOINT;

    if (strlen(nmea_payload) != 59) {  status = AIS_ERR_BAD_BIT_COUNT; return;  }

    std::bitset<354> bs;  // 352 + 2 spares to be 6 bit aligned
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;  // checks status
    // CHECKPOINT;

    message_id = ubits(bs, 0, 6);
    if (8 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    spare = ubits(bs,38,2);
    dac = ubits(bs,40,10);
    fi = ubits(bs,50,6);

    //CHECKPOINT;
    // FIX: if dac is not 001, it could still possibly be correct
    if ( 1 != dac || 11 != fi ) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    
    y = sbits(bs,56,24) / 60000.; // YES, lat is first
    x = sbits(bs,80,25) / 60000.;
    day = ubits(bs,105,5);
    hour = ubits(bs,110,5);
    minute = ubits(bs,115,6);
    wind_ave = ubits(bs,121,7);
    wind_gust = ubits(bs,128,7);
    wind_dir = ubits(bs,135,9);
    wind_gust_dir = ubits(bs,144,9);
    air_temp = ubits(bs,153,11) / 10. - 60;
    rel_humid = ubits(bs,164,7);
    dew_point = ubits(bs,171,10) / 10. - 20; // FIX: please be right
    air_pres = ubits(bs,181,9) + 800;
    air_pres_trend = ubits(bs,190,2);
    horz_vis = ubits(bs,192,8) / 10.;
    water_level = ubits(bs,200,9) / 10. - 10; // FIX: please be right for -10.0 to 30.0
    water_level_trend = ubits(bs,209,2);
    surf_cur_speed = ubits(bs,211,8) / 10.;
    surf_cur_dir = ubits(bs,219,9);
    cur_speed_2 = ubits(bs,228,8) / 10.;
    cur_dir_2 = ubits(bs,236,9);
    cur_depth_2 = ubits(bs,245,5);
    cur_speed_3 = ubits(bs,250,8) / 10.;
    cur_dir_3 = ubits(bs,258,9);
    cur_depth_3 = ubits(bs,267,5);

    wave_height = ubits(bs,272,8) / 10.;
    wave_period = ubits(bs,280,6);
    wave_dir = ubits(bs,286,9);
    swell_height = ubits(bs,295,8) / 10.;
    swell_period = ubits(bs,303,6);
    swell_dir = ubits(bs,309,9);

    sea_state = ubits(bs,318,4);
    water_temp = ubits(bs,322,10) / 10. - 10; // FIX: please be right for -10.0 to +50.0
    precip_type = ubits(bs,332,3);
    salinity = ubits(bs,335,9);
    ice = ubits(bs,344,2);
    extended_water_level = ubits(bs,346,6); // FIX: how to treat this???
}

void Ais8_1_11::print() {
    std::cout << "BBM_imo_1_11_met_hydro: " << message_id 
              << "\t\tdac: " << dac << "\tfi:" << fi << "\n";

    std::cout << "\tspare:" << spare << "\n"
              << "\tlocation:" <<  x << " " <<  y << "\t(lon, lat WGS84)" << "\n"
              << "\ttime:" <<  day << "T" << hour << ":" << minute << "Z\n"
              << "\twind:" <<  wind_ave << " " << wind_dir << "\n"
              << "\tgust:" <<  wind_gust << " " << wind_gust_dir << "\n"
              << "\tair_temp:" <<  air_temp << "\thumidity:" <<  rel_humid << "\t:" <<  horz_vis << "\n"
              << "\tdew_point:" <<  dew_point << "\tair_pres:" <<  air_pres << " trend: " << air_pres_trend << "\n"
              << "\twater_level:" <<  water_level << " trend: " << water_level_trend << "\n"
              << "\tcurrent:" <<  surf_cur_speed << " kts dir: " << surf_cur_dir << "\n"
              << "\tcurr2:" <<  cur_speed_2 << " kts dir: " << cur_dir_2 << "\t " <<  cur_depth_2 << " m deep\n" 
              << "\tcurr3:" <<  cur_speed_3 << " kts dir: " << cur_dir_3 << "\t " <<  cur_depth_3 << " m deep\n" 
              << "\twaves:" <<  wave_height << " " << wave_period << " " <<  wave_dir << "\n"
              << "\tswell:" <<  swell_height << " " <<  swell_period << " " <<  swell_dir << "\n"
              << "\tsea_state:" <<  sea_state << "\twater_temp:" <<  water_temp << "\n"
              << "\tprecip_type:" <<  precip_type << "\tsalinity:" <<  salinity << "\n"
              << "\tice:" <<  ice << "\n" 
              << "\tspare_or_extended_wl:" <<  extended_water_level << std::endl;
}
