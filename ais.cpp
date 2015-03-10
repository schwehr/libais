// General helper functions for working with AIS and NMEA.

#include "ais.h"

namespace libais {

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

// Note: Needs to be kept in sync with enum AIS_STATUS list in ais.h.
const char * const AIS_STATUS_STRINGS[AIS_STATUS_NUM_CODES] = {
  "AIS_UNINITIALIZED",
  "AIS_OK",
  "AIS_ERR_BAD_BIT_COUNT",
  "AIS_ERR_BAD_NMEA_CHR",
  "AIS_ERR_BAD_PTR",
  "AIS_ERR_UNKNOWN_MSG_TYPE",
  "AIS_ERR_MSG_NOT_IMPLEMENTED",
  "AIS_ERR_MSG_SUB_NOT_IMPLEMENTED",
  "AIS_ERR_EXPECTED_STRING",
  "AIS_ERR_BAD_MSG_CONTENT",
  "AIS_ERR_MSG_TOO_LONG",
  "AIS_ERR_BAD_SUB_MSG",
  "AIS_ERR_BAD_SUB_SUB_MSG",
};


AisBitset::AisBitset() : num_bits(0), num_chars(0), current_position(0) {}

AIS_STATUS AisBitset::ParseNmeaPayload(const char *nmea_payload, int pad) {
  InitNmeaOrd();

  num_bits = 0;
  num_chars = 0;
  current_position = 0;
  reset();

  assert(nmea_payload);
  assert(pad >= 0 && pad < 6);

  if (strlen(nmea_payload) > size()/6) {
#ifndef NDEBUG
    std::cerr << "ERROR: message longer than max allowed size (" << size()/6
              << "): found " << strlen(nmea_payload) << " characters in "
              << nmea_payload << std::endl;
#endif
    return AIS_ERR_MSG_TOO_LONG;
  }
  for (size_t idx = 0; nmea_payload[idx] != '\0' && idx < size()/6; idx++) {
    int c = static_cast<int>(nmea_payload[idx]);
    if (c < 48 || c > 119 || (c >= 88 && c <= 95)) {
      return AIS_ERR_BAD_NMEA_CHR;
    }
    for (size_t offset = 0; offset < 6; offset++) {
      set(idx*6 + offset, nmea_ord_[c].test(offset));
    }
  }
  num_chars = strlen(nmea_payload);
  num_bits = num_chars * 6 - pad;

  return AIS_OK;
}

const AisBitset& AisBitset::SeekRelative(int d) const {
  assert((current_position + d) >= 0 && (current_position + d) < num_bits);
  current_position += d;
  return *this;
}

const AisBitset& AisBitset::SeekTo(size_t pos) const {
  assert(pos < num_bits);
  current_position = pos;
  return *this;
}

bool AisBitset::operator[](size_t pos) const {
  // TODO(schwehr): Prefer to use num_bits (includes pad) for checking bounds.
  assert(pos < num_chars * 6);

  assert(current_position == pos);

  current_position = pos + 1;
  return bitset<MAX_BITS>::operator[](pos);
}

unsigned int AisBitset::ToUnsignedInt(const size_t start,
                                      const size_t len) const {
  assert(len <= 32);
  // TODO(schwehr): Prefer to use num_bits (includes pad) for checking bounds.
  assert(start + len <= num_chars * 6);

  assert(current_position == start);

  bitset<32> bs_tmp;
  for (size_t i = 0; i < len; i++)
    bs_tmp.set(i, test(start + len - i - 1));

  current_position = start + len;

  return bs_tmp.to_ulong();
}

int AisBitset::ToInt(const size_t start, const size_t len)  const {
  assert(len <= 32);
  // TODO(schwehr): Prefer to use num_bits (includes pad) for checking bounds.
  assert(start + len <= num_chars * 6);

  assert(current_position == start);

  bitset<32> bs32;
  // pad 1's to the left if negative
  if (len < 32 && test(start))
    bs32.flip();

  for (size_t i = 0; i < len; i++)
    bs32[i] = test(start + len - i - 1);

  long_union val;
  val.ulong_val = bs32.to_ulong();

  current_position = start + len;

  return val.long_val;
}

string AisBitset::ToString(const size_t start, const size_t len) const {
  assert(len % 6 == 0);
  // TODO(schwehr): Prefer to use num_bits (includes pad) for checking bounds.
  assert(start + len <= num_chars * 6);

  assert(current_position == start);

  const size_t num_char = len / 6;
  string result(num_char, '@');
  for (size_t char_idx = 0; char_idx < num_char; char_idx++) {
    const int char_num = ToUnsignedInt(start + char_idx*6, 6);
    result[char_idx] = bits_to_char_tbl_[char_num];
  }
  return result;
}

// static private

void AisBitset::InitNmeaOrd() {
  if (nmea_ord_initialized_) {
    return;
  }

  for (int c = 48; c < 88; c++) {
    int val = c - 48;
    nmea_ord_[c] = Reverse(bitset<6>(val));
  }
  for (int c = 88; c < 128; c++) {
    int val = c - 48;
    val -= 8;
    nmea_ord_[c] = Reverse(bitset<6>(val));
  }
  nmea_ord_initialized_ = true;
}

bitset<6> AisBitset::Reverse(const bitset<6> &bits) {
  bitset<6> out;
  for (size_t i = 0; i < 6; i++)
    out[5 - i] = bits[i];
  return out;
}

bool AisBitset::nmea_ord_initialized_ = false;
bitset<6> AisBitset::nmea_ord_[128];
// For decoding str bits inside of a binary message.
const char AisBitset::bits_to_char_tbl_[] = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "[\\]^- !\"#$%&`()*+,-./0123456789:;<=>?";


AisMsg::AisMsg(const char *nmea_payload, const size_t pad)
    : message_id(0), repeat_indicator(0), mmsi(0), status(AIS_UNINITIALIZED) {
  assert(nmea_payload);
  assert(pad < 6);

  AisBitset bs;
  const AIS_STATUS r = bs.ParseNmeaPayload(nmea_payload, pad);
  if (r != AIS_OK) {
    status = r;
    return;
  }
  num_bits = bs.GetNumBits();
  num_chars = bs.GetNumChars();

  if (bs.GetNumBits() < 38) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  message_id = bs.ToUnsignedInt(0, 6);
  repeat_indicator = bs.ToUnsignedInt(6, 2);
  mmsi = bs.ToUnsignedInt(8, 30);
}

}  // namespace libais
