// 'J' - Multi slot binary message with comm state - TODO: handle payload
// BAD: the comm-state is after the veriable payload.  This is a bad design.
#include "ais.h"

Ais26::Ais26(const char *nmea_payload, const size_t pad) {
    assert(nmea_payload);
    init();

    const size_t num_bits = strlen(nmea_payload) * 6 - pad;
    const size_t comm_flag_offset = num_bits - 20;
    // const int num_char = std::strlen(nmea_payload);

    if (52 > num_bits || num_bits > 1064) { status = AIS_ERR_BAD_BIT_COUNT; return; }

    std::bitset<1064> bs;
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (26 != message_id) {status = AIS_ERR_WRONG_MSG_TYPE; return;}
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    const bool addressed = bs[38];
    use_app_id = bs[39];
    if (addressed) {
      dest_mmsi_valid = true;
      dest_mmsi = ubits(bs, 40, 30);

      if (use_app_id) {
        dac = ubits(bs,70,10);
        fi = ubits(bs,80,6);
      }
      // TODO: deal with payload

    } else {
      // broadcast
      if (use_app_id) {
        dac = ubits(bs,40,10);
        fi = ubits(bs,50,6);
      }

      // TODO: deal with payload - probably need to pass in the spare bits
    }

    commstate_flag = bs[comm_flag_offset];
    sync_state = ubits(bs, comm_flag_offset+1 , 2); // Both SOTDMA and TDMA

    if (not commstate_flag) {
      // SOTDMA
      slot_timeout = ubits(bs, comm_flag_offset+3 ,3);
      slot_timeout_valid = true;
      switch (slot_timeout) {
        case 0:
          slot_offset = ubits(bs, comm_flag_offset+6, 14);
          slot_offset_valid = true;
          break;
        case 1:
          utc_hour = ubits(bs, comm_flag_offset+6 , 5);
          utc_min = ubits(bs, comm_flag_offset+11 , 7);
          utc_spare = ubits(bs, comm_flag_offset+18 , 2);
          utc_valid = true;
          break;
        case 2: // FALLTHROUGH
        case 4: // FALLTHROUGH
        case 6:
          slot_number = ubits(bs, comm_flag_offset+6 , 14);
          slot_number_valid = true;
          break;
        case 3: // FALLTHROUGH
        case 5: // FALLTHROUGH
        case 7:
          received_stations = ubits(bs, comm_flag_offset+6 , 14);
          received_stations_valid = true;
          break;
        default:
          assert (false);
      }
    } else {
      // ITDMA
      slot_increment = ubits(bs, comm_flag_offset+3, 13);
      slot_increment_valid = true;

      slots_to_allocate = ubits(bs, comm_flag_offset+16, 3);
      slots_to_allocate_valid = true;

      keep_flag = bool(bs[comm_flag_offset+19]);
      keep_flag_valid = true;
    }

}


void Ais26::print() {
    //CHECKPOINT;
    std::cout << "Multi-slot slot binary message w/ comm state: " << message_id << "\n"
              << "\tmmsi: " << mmsi << " repeat: " << repeat_indicator << "\n";
    // TODO: finish
}
