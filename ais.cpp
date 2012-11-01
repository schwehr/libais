#include "ais.h"

const string nth_field(const string &str, const size_t n, const char c) {
  if (!n) {
    const size_t pos = str.find(c);
    if (pos == string::npos)
      return string("");
    return str.substr(0, pos);
  }

  size_t pos = 0;
  for (size_t count = 0; count < n && pos != string::npos; count++) {
    if (pos > 0) pos += 1;
    pos = str.find(c, pos);
  }
  if (string::npos == pos)
    return string("");

  const size_t start = pos;
  const size_t end = str.find(c, pos+1);
  if (string::npos == end)
    return str.substr(start+1);
  return str.substr(start+1, end-start-1);
}

// for decoding str bits inside of a binary message
const string bits_to_char_tbl="@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^- !\"#$%&`()*+,-./0123456789:;<=>?";

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

bitset<6> reverse(const bitset<6> &bits) {
  bitset<6> out;
  for (size_t i = 0; i < 6; i++)
    out[5-i] = bits[i];
  return out;
}

bitset<6> nmea_ord[128];
bool nmea_ord_initialized = false;

// TODO(schwehr): singleton
// http://gpsd.berlios.de/AIVDM.html#_aivdm_aivdo_payload_armoring
void build_nmea_lookup() {
  for (int c = 48; c < 128; c++) {
    int val = c - 48;
    if (val >= 40) val-= 8;
    if (val < 0) continue;
    nmea_ord[c] = reverse(bitset<6>(val));
  }
  nmea_ord_initialized = true;
}
