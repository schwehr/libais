#include "ais.h"

vector<size_t> delimiters(const string &str,
                                const string &delim_str) {
  vector<size_t> r;
  if (str.size() == 0)
    return r;
  for (size_t off = str.find(delim_str);
      off != string::npos;
      off = str.find(delim_str, off + 1)) {
    r.push_back(off);
  }
  return r;
}

vector<string> split(const string &str, const string &delim_str) {
  vector<string> r;
  vector<size_t> delims = delimiters(str, delim_str);
  if (!delims.size()) {
    r.push_back(str);
    return r;
  }
  r.push_back(str.substr(0, delims[0]));
  if (delims.size() > 1) {
    for (size_t d = 1; d < delims.size(); d++) {
      const size_t start = delims[d - 1]+delim_str.size();
      const size_t len = delims[d] - start;
      r.push_back(str.substr(start, len));
    }
  }
  r.push_back(str.substr(delims[delims.size()-1]+delim_str.size()));
  return r;
}

string nth_field(const string &str, const size_t n,
                       const string &delim_str) {
  const vector<size_t> delims = delimiters(str, delim_str);
  if (n > delims.size()) {
    return "";
  }
  if (n == 0) {
    if (delims.size() > 0)
      return str.substr(0, delims[0]);
    else
      return str;
  }
  if (n == delims.size())
    return str.substr(delims[delims.size() - 1] + delim_str.size());
  const size_t start = delims[n - 1]+delim_str.size();
  const size_t len = delims[n] - start;
  return str.substr(start, len);
}

// for decoding str bits inside of a binary message
const string bits_to_char_tbl="@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "[\\]^- !\"#$%&`()*+,-./0123456789:;<=>?";

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
    out[5 - i] = bits[i];
  return out;
}

bitset<6> nmea_ord[128];
bool nmea_ord_initialized = false;

// TODO(schwehr): singleton
// http://gpsd.berlios.de/AIVDM.html#_aivdm_aivdo_payload_armoring
void build_nmea_lookup() {
  for (int c = 48; c < 128; c++) {
    int val = c - 48;
    if (val >= 40)
      val -= 8;
    nmea_ord[c] = reverse(bitset<6>(val));
  }
  nmea_ord_initialized = true;
}
