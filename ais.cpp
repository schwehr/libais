// General helper functions for working with AIS and NMEA.

#include "ais.h"

vector<string> Split(const string &str, const string &delim_str) {
  assert(!delim_str.empty());
  vector<string> r;
  if (str.empty()) {
    r.push_back("");
    return r;
  }
  size_t prev = 0;
  for (size_t off = str.find(delim_str);
      off != string::npos;
      off = str.find(delim_str, off + 1)) {
    r.push_back(str.substr(prev, off-prev));
    prev = off + delim_str.size();
  }
  r.push_back(str.substr(prev));
  return r;
}

string GetNthField(const string &str, const size_t n,
                   const string &delim_str) {
  assert(!delim_str.empty());
  if (str.empty())
    return "";

  size_t prev = 0;
  size_t off = str.find(delim_str);
  size_t count = 0;

  for (; off != string::npos && count != n;
       off = str.find(delim_str, off + 1), count++) {
    prev = off + delim_str.size();
  }

  if (count == n)
    return str.substr(prev, off-prev);

  return "";
}

int GetPad(const string &nmea_str) {
  // -1 on error
  const string chksum_block(GetNthField(nmea_str, 6, ","));
  if (chksum_block.size() != 4 || chksum_block[1] != '*')
    return -1;
  const char pad = chksum_block[0];
  if (pad < '0' || pad > '5')
    return -1;
  return static_cast<int>(pad - '0');
}

string GetBody(const string &nmea_str) {
  return GetNthField(nmea_str, 5, ",");
}

// for decoding str bits inside of a binary message
const string bits_to_char_tbl="@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "[\\]^- !\"#$%&`()*+,-./0123456789:;<=>?";

const char * const AIS_STATUS_STRINGS[AIS_STATUS_NUM_CODES] = {
  "AIS_UNINITIALIZED",
  "AIS_OK",
  "AIS_ERR_BAD_BIT_COUNT",
  "AIS_ERR_BAD_NMEA_CHR",
  "AIS_ERR_BAD_PTR",
  "AIS_ERR_UNKNOWN_MSG_TYPE",
  "AIS_ERR_MSG_NOT_IMPLEMENTED",
  "AIS_ERR_BAD_MSG_CONTENT",
  "AIS_ERR_EXPECTED_STRING",
  "AIS_ERR_MSG_TOO_LONG",
};

bitset<6> Reverse(const bitset<6> &bits) {
  bitset<6> out;
  for (size_t i = 0; i < 6; i++)
    out[5 - i] = bits[i];
  return out;
}

bitset<6> nmea_ord[128];
bool nmea_ord_initialized = false;

// TODO(schwehr): singleton
// http://gpsd.berlios.de/AIVDM.html#_aivdm_aivdo_payload_armoring
void BuildNmeaLookup() {
  if (nmea_ord_initialized)
    return;
  for (int c = 48; c < 88; c++) {
    int val = c - 48;
    nmea_ord[c] = Reverse(bitset<6>(val));
  }
  for (int c = 88; c < 128; c++) {
    int val = c - 48;
    val -= 8;
    nmea_ord[c] = Reverse(bitset<6>(val));
  }
  nmea_ord_initialized = true;
}

AisMsg::AisMsg(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  assert(pad < 6);
  assert(nmea_ord_initialized);
  status = AIS_UNINITIALIZED;

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  if (num_bits < 38) {
    std::cerr << "AisMsg num_bits " << num_bits << "\n";
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<42> bs;
  const string header(nmea_payload, 7);
  const AIS_STATUS r = aivdm_to_bits(bs, header.c_str());
  if (r != AIS_OK) {
    status = r;
    return;
  }

  message_id = ubits(bs, 0, 6);
  repeat_indicator = ubits(bs, 6, 2);
  mmsi = ubits(bs, 8, 30);
}
