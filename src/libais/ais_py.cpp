// TODO(schwehr): check for reference counting leaks
// TODO(schwehr): better error handling for all messages
#include <cassert>
#include <cstddef>
#include <cstddef>
#include <string>

#include <Python.h>

#include "ais.h"

namespace libais {

PyObject *ais_py_exception;
const char exception_short[] = "DecodeError";
const char exception_name[] = "_ais.DecodeError";

// TODO(schwehr): Write a full module doc string.

// Functional Identifiers (FI) are individual messages within a
// specific DAC.  An FI in one DAC has nothing to do with an FI in
// another DAC.
enum AIS_FI {
  AIS_FI_6_1_0_TEXT = 0,
  AIS_FI_6_1_1_ACK = 1,
  AIS_FI_6_1_2_FI_INTERROGATE = 2,
  AIS_FI_6_1_3_CAPABILITY_INTERROGATE = 3,
  AIS_FI_6_1_4_CAPABILITY_REPLY = 4,
  AIS_FI_6_1_5_ACK = 5,
  AIS_FI_6_1_12_DANGEROUS_CARGO = 12,
  AIS_FI_6_1_14_TIDAL_WINDOW = 14,
  AIS_FI_6_1_16_VTS_TARGET = 16,
  AIS_FI_6_1_18_ENTRY_TIME = 18,
  AIS_FI_6_1_20_BERTHING = 20,
  AIS_FI_6_1_25_DANGEROUS_CARGO = 25,
  AIS_FI_6_1_28_ROUTE = 28,
  AIS_FI_6_1_30_TEXT = 30,
  AIS_FI_6_1_32_TIDAL_WINDOW = 32,
  AIS_FI_6_1_40_PERSONS_ON_BOARD = 40,
  AIS_FI_6_200_21_RIS_VTS_ETA = 21,
  AIS_FI_6_200_22_RIS_VTS_RTA = 22,
  AIS_FI_6_200_55_RIS_VTS_SAR = 55,

  AIS_FI_8_1_0_TEXT = 0,
  AIS_FI_8_1_11_MET_HYDRO = 11,
  AIS_FI_8_1_13_FAIRWAY_CLOSED = 13,
  AIS_FI_8_1_15_SHIP_AND_VOYAGE = 15,
  AIS_FI_8_1_16_PERSONS_ON_BOARD = 16,
  AIS_FI_8_1_17_VTS_TARGET = 17,
  AIS_FI_8_1_19_TRAFFIC_SIGNAL = 19,
  AIS_FI_8_1_21_WEATHER_OBS = 21,
  AIS_FI_8_1_22_AREA_NOTICE = 22,
  AIS_FI_8_1_24_SHIP_AND_VOYAGE = 24,
  AIS_FI_8_1_26_SENSOR = 26,
  AIS_FI_8_1_27_ROUTE = 27,
  AIS_FI_8_1_29_TEXT = 29,
  AIS_FI_8_1_31_MET_HYDRO = 31,
  AIS_FI_8_1_40_PERSONS_ON_BOARD = 40,
  AIS_FI_8_200_10_RIS_SHIP_AND_VOYAGE = 10,
  AIS_FI_8_200_21_RIS_ETA_AT_LOCK_BRIDGE_TERMINAL = 21,
  AIS_FI_8_200_22_RIS_RTA_AT_LOCK_BRIDGE_TERMINAL = 22,
  AIS_FI_8_200_23_RIS_EMMA_WARNING = 23,
  AIS_FI_8_200_24_RIS_WATERLEVEL = 24,
  AIS_FI_8_200_40_RIS_ATON_SIGNAL_STATUS = 40,
  AIS_FI_8_200_55_RIS_PERSONS_ON_BOARD = 50,
  AIS_FI_8_366_22_AREA_NOTICE = 22,  // USCG.
  AIS_FI_8_367_22_AREA_NOTICE = 22,  // USCG.
  AIS_FI_8_367_23_SSW = 23,  // USCG Satellite Ship Weather
  AIS_FI_8_367_24_SSW_SMALL = 24,  // USCG Satellite Ship Weather Small
  AIS_FI_8_367_25_SSW_TINY = 25,  // USCG Satellite Ship Weather Tiny
  AIS_FI_8_367_33_ENVIRONMENTAL = 33, // Environmental Message
};

void
DictSafeSetItem(
    PyObject *dict, const std::string &key, const long val) {  // NOLINT
  PyObject *val_obj = PyLong_FromLong(val);
  assert(val_obj);
  PyDict_SetItemString(dict, key.c_str(), val_obj);
  Py_DECREF(val_obj);
}

void
DictSafeSetItem(PyObject *dict, const std::string &key, const int val) {
  PyObject *val_obj = PyLong_FromLong(val);
  assert(val_obj);
  PyDict_SetItemString(dict, key.c_str(), val_obj);
  Py_DECREF(val_obj);
}

void
DictSafeSetItem(
    PyObject *dict, const std::string &key, const unsigned int val) {
  PyObject *val_obj = PyLong_FromLong(val);
  assert(val_obj);
  PyDict_SetItemString(dict, key.c_str(), val_obj);
  Py_DECREF(val_obj);
}

void
DictSafeSetItem(PyObject *dict, const std::string &key, const std::string &val) {
  PyObject *val_obj = PyUnicode_FromString(val.c_str());
  assert(val_obj);
  PyDict_SetItemString(dict, key.c_str(), val_obj);
  Py_DECREF(val_obj);
}


void
DictSafeSetItem(PyObject *dict, const std::string &key, const char *val) {
  PyObject *val_obj = PyUnicode_FromString(val);
  assert(val_obj);
  PyDict_SetItemString(dict, key.c_str(), val_obj);
  Py_DECREF(val_obj);
}


#if 0
void
DictSafeSetItem(PyObject *dict, const std::string &key, const bool val) {
  PyObject *key_obj = PyUnicode_FromString(key.c_str());
  PyObject *val_obj = PyBool_FromLong(val);
  assert(key_obj);
  assert(val_obj);
  PyDict_SetItem(dict, key_obj, val_obj);
  Py_DECREF(key_obj);
  Py_DECREF(val_obj);
}
#else
void
DictSafeSetItem(PyObject *dict, const std::string &key, const bool val) {
  if (val) {
    PyDict_SetItemString(dict, key.c_str(), Py_True);
  } else {
    PyDict_SetItemString(dict, key.c_str(), Py_False);
  }
}
#endif

void
DictSafeSetItem(PyObject *dict, const std::string &key, const float val) {
  PyObject *val_obj = PyFloat_FromDouble(val);
  assert(val_obj);
  PyDict_SetItemString(dict, key.c_str(), val_obj);
  Py_DECREF(val_obj);
}

// Python Floats are IEE-754 double precision.
void
DictSafeSetItem(PyObject *dict, const std::string &key, const double val) {
  PyObject *val_obj = PyFloat_FromDouble(val);
  assert(val_obj);
  PyDict_SetItemString(dict, key.c_str(), val_obj);
  Py_DECREF(val_obj);
}

void
DictSafeSetItem(PyObject *dict, const std::string &x_key, const std::string& y_key,
                const AisPoint& position) {
  DictSafeSetItem(dict, x_key, position.lng_deg);
  DictSafeSetItem(dict, y_key, position.lat_deg);
}

void
DictSafeSetItem(PyObject *dict, const std::string &key, PyObject *val_obj) {
  // When we need to add dictionaries and such to a dictionary
  assert(dict);
  assert(val_obj);
  PyDict_SetItemString(dict, key.c_str(), val_obj);
}


PyObject *
ais_msg_to_pydict(const AisMsg* msg) {
  assert(msg);

  PyObject *dict = PyDict_New();
  DictSafeSetItem(dict, "id", msg->message_id);
  DictSafeSetItem(dict, "repeat_indicator", msg->repeat_indicator);
  DictSafeSetItem(dict, "mmsi", msg->mmsi);

  return dict;
}

// Class A position report
PyObject *
ais1_2_3_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);

  Ais1_2_3 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais1_2_3: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  DictSafeSetItem(dict, "nav_status", msg.nav_status);
  DictSafeSetItem(dict, "rot_over_range", msg.rot_over_range);
  DictSafeSetItem(dict, "rot", msg.rot);
  DictSafeSetItem(dict, "sog", msg.sog);
  DictSafeSetItem(dict, "position_accuracy", msg.position_accuracy);
  DictSafeSetItem(dict, "x", "y", msg.position);
  DictSafeSetItem(dict, "cog", msg.cog);
  DictSafeSetItem(dict, "true_heading", msg.true_heading);
  DictSafeSetItem(dict, "timestamp", msg.timestamp);
  DictSafeSetItem(dict, "special_manoeuvre", msg.special_manoeuvre);
  DictSafeSetItem(dict, "spare", msg.spare);
  DictSafeSetItem(dict, "raim", msg.raim);

  // COMM States
  DictSafeSetItem(dict, "sync_state", msg.sync_state);  // Both SOTDMA & ITDMA

  // SOTDMA
  if (msg.message_id == 1 || msg.message_id == 2) {
    if (msg.slot_timeout_valid) {
      DictSafeSetItem(dict, "slot_timeout", msg.slot_timeout);
    }

    if (msg.received_stations_valid) {
      DictSafeSetItem(dict, "received_stations", msg.received_stations);
    }
    if (msg.slot_number_valid) {
      DictSafeSetItem(dict, "slot_number", msg.slot_number);
    }
    if (msg.utc_valid) {
      DictSafeSetItem(dict, "utc_hour", msg.utc_hour);
      DictSafeSetItem(dict, "utc_min", msg.utc_min);
      DictSafeSetItem(dict, "utc_spare", msg.utc_spare);
    }

    if (msg.slot_offset_valid) {
      DictSafeSetItem(dict, "slot_offset", msg.slot_offset);
    }
  }

  // ITDMA
  if (msg.slot_increment_valid) {
    DictSafeSetItem(dict, "slot_increment", msg.slot_increment);
    DictSafeSetItem(dict, "slots_to_allocate", msg.slots_to_allocate);
    DictSafeSetItem(dict, "keep_flag", msg.keep_flag);
  }

  return dict;
}

// Basestation report and ';' time report
PyObject *
ais4_11_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);

  Ais4_11 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais4_11: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  DictSafeSetItem(dict, "year", msg.year);
  DictSafeSetItem(dict, "month", msg.month);
  DictSafeSetItem(dict, "day", msg.day);
  DictSafeSetItem(dict, "hour", msg.hour);
  DictSafeSetItem(dict, "minute", msg.minute);
  DictSafeSetItem(dict, "second", msg.second);

  DictSafeSetItem(dict, "position_accuracy", msg.position_accuracy);
  DictSafeSetItem(dict, "x", "y", msg.position);

  DictSafeSetItem(dict, "fix_type", msg.fix_type);
  DictSafeSetItem(dict, "transmission_ctl", msg.transmission_ctl);
  DictSafeSetItem(dict, "spare", msg.spare);
  DictSafeSetItem(dict, "raim", msg.raim);

  // SOTDMA
  DictSafeSetItem(dict, "sync_state", msg.sync_state);
  DictSafeSetItem(dict, "slot_timeout", msg.slot_timeout);

  if (msg.received_stations_valid)
    DictSafeSetItem(dict, "received_stations", msg.received_stations);
  if (msg.slot_number_valid)
    DictSafeSetItem(dict, "slot_number", msg.slot_number);
  if (msg.utc_valid) {
    DictSafeSetItem(dict, "utc_hour", msg.utc_hour);
    DictSafeSetItem(dict, "utc_min", msg.utc_min);
    DictSafeSetItem(dict, "utc_spare", msg.utc_spare);
  }

  if (msg.slot_offset_valid)
    DictSafeSetItem(dict, "slot_offset", msg.slot_offset);

  return dict;
}

// Class A ship data
PyObject *
ais5_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  Ais5 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais5: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  DictSafeSetItem(dict, "ais_version", msg.ais_version);
  DictSafeSetItem(dict, "imo_num", msg.imo_num);
  DictSafeSetItem(dict, "callsign", msg.callsign);
  DictSafeSetItem(dict, "name", msg.name);
  DictSafeSetItem(dict, "type_and_cargo", msg.type_and_cargo);
  DictSafeSetItem(dict, "dim_a", msg.dim_a);
  DictSafeSetItem(dict, "dim_b", msg.dim_b);
  DictSafeSetItem(dict, "dim_c", msg.dim_c);
  DictSafeSetItem(dict, "dim_d", msg.dim_d);
  DictSafeSetItem(dict, "fix_type", msg.fix_type);
  DictSafeSetItem(dict, "eta_month", msg.eta_month);
  DictSafeSetItem(dict, "eta_day", msg.eta_day);
  DictSafeSetItem(dict, "eta_hour", msg.eta_hour);
  DictSafeSetItem(dict, "eta_minute", msg.eta_minute);
  DictSafeSetItem(dict, "draught", msg.draught);
  DictSafeSetItem(dict, "destination", msg.destination);
  DictSafeSetItem(dict, "dte", msg.dte);
  DictSafeSetItem(dict, "spare", msg.spare);

  return dict;
}

// Address binary message
AIS_STATUS
ais6_1_0_append_pydict(const char *nmea_payload, PyObject *dict,
                       const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais6_1_0 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "ack_required",  msg.ack_required);
  DictSafeSetItem(dict, "msg_seq",  msg.msg_seq);
  DictSafeSetItem(dict, "text", msg.text);
  DictSafeSetItem(dict, "spare2", msg.spare2);

  return AIS_OK;
}

AIS_STATUS
ais6_1_1_append_pydict(const char *nmea_payload, PyObject *dict,
                       const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais6_1_1 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "ack_dac", msg.ack_dac);
  DictSafeSetItem(dict, "msg_seq", msg.msg_seq);
  DictSafeSetItem(dict, "spare2", msg.spare2);

  return AIS_OK;
}

AIS_STATUS
ais6_1_2_append_pydict(const char *nmea_payload, PyObject *dict,
                       const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais6_1_2 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "req_dac", msg.req_dac);
  DictSafeSetItem(dict, "req_fi", msg.req_fi);

  return AIS_OK;
}

AIS_STATUS
ais6_1_3_append_pydict(const char *nmea_payload, PyObject *dict,
                       const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);

  Ais6_1_3 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "req_dac", msg.req_dac);
  DictSafeSetItem(dict, "spare2", msg.spare2);
  DictSafeSetItem(dict, "spare3", msg.spare3);
  DictSafeSetItem(dict, "spare4", msg.spare4);

  return AIS_OK;
}

AIS_STATUS
ais6_1_4_append_pydict(const char *nmea_payload, PyObject *dict,
                       const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais6_1_4 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "ack_dac", msg.ack_dac);
  constexpr int kNumFI = 64;
  PyObject *cap_list = PyList_New(kNumFI);
  PyObject *res_list = PyList_New(kNumFI);
  for (size_t cap_num = 0; cap_num < kNumFI; cap_num++) {
    PyObject *cap = PyLong_FromLong(long(msg.capabilities[cap_num]));  // NOLINT
    PyList_SetItem(cap_list, cap_num, cap);

    PyObject *res = PyLong_FromLong(long(msg.cap_reserved[cap_num]));  // NOLINT
    PyList_SetItem(res_list, cap_num, res);
  }
  DictSafeSetItem(dict, "capabilities", cap_list);
  Py_DECREF(cap_list);
  DictSafeSetItem(dict, "cap_reserved", res_list);
  Py_DECREF(res_list);
  DictSafeSetItem(dict, "spare2", msg.spare2);
  DictSafeSetItem(dict, "spare3", msg.spare2);
  DictSafeSetItem(dict, "spare4", msg.spare2);
  DictSafeSetItem(dict, "spare5", msg.spare2);

  return AIS_OK;
}

AIS_STATUS
ais6_1_5_append_pydict(const char *nmea_payload, PyObject *dict,
                       const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais6_1_5 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "ack_dac", msg.ack_dac);
  DictSafeSetItem(dict, "ack_fi", msg.ack_dac);
  DictSafeSetItem(dict, "seq_num", msg.ack_dac);
  DictSafeSetItem(dict, "ai_available", msg.ack_dac);
  DictSafeSetItem(dict, "ai_response", msg.ack_dac);
  DictSafeSetItem(dict, "spare", msg.ack_dac);

  return AIS_OK;
}

AIS_STATUS
ais6_1_12_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(pad < 6);
  Ais6_1_12 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "last_port", msg.last_port);
  DictSafeSetItem(dict, "utc_month_dep", msg.utc_month_dep);  // actual
  DictSafeSetItem(dict, "utc_day_dep", msg.utc_day_dep);
  DictSafeSetItem(dict, "utc_hour_dep", msg.utc_hour_dep);
  DictSafeSetItem(dict, "utc_min_dep", msg.utc_min_dep);
  DictSafeSetItem(dict, "next_port", msg.next_port);
  DictSafeSetItem(dict, "utc_month_next", msg.utc_month_next);  // estimated
  DictSafeSetItem(dict, "utc_day_next", msg.utc_day_next);
  DictSafeSetItem(dict, "utc_hour_next", msg.utc_hour_next);
  DictSafeSetItem(dict, "utc_min_next", msg.utc_min_next);
  DictSafeSetItem(dict, "main_danger", msg.main_danger);
  DictSafeSetItem(dict, "imo_cat", msg.imo_cat);
  DictSafeSetItem(dict, "un", msg.un);
  DictSafeSetItem(dict, "value", msg.value);  // TODO(schwehr): units?
  DictSafeSetItem(dict, "value_unit", msg.value_unit);
  DictSafeSetItem(dict, "spare2", msg.spare2);

  return AIS_OK;
}

// 6_1_13 does not exist

// IMO Circ 289 - Tidal Window
AIS_STATUS
ais6_1_14_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(pad < 6);
  Ais6_1_14 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "utc_month", msg.utc_month);
  DictSafeSetItem(dict, "utc_day", msg.utc_day);

  if (!msg.windows.size())
    return AIS_ERR_BAD_SUB_MSG;

  PyObject *window_list = PyList_New(msg.windows.size());
  for (size_t w_num = 0; w_num < msg.windows.size(); w_num++) {
    PyObject *window = PyDict_New();
    DictSafeSetItem(window, "x", "y", msg.windows[w_num].position);
    DictSafeSetItem(window, "utc_hour_from", msg.windows[w_num].utc_hour_from);
    DictSafeSetItem(window, "utc_min_from", msg.windows[w_num].utc_min_from);
    DictSafeSetItem(window, "utc_hour_to", msg.windows[w_num].utc_hour_to);
    DictSafeSetItem(window, "utc_min_to", msg.windows[w_num].utc_min_to);
    DictSafeSetItem(window, "cur_dir", msg.windows[w_num].cur_dir);
    DictSafeSetItem(window, "cur_speed", msg.windows[w_num].cur_speed);
    PyList_SetItem(window_list, w_num, window);
  }
  PyDict_SetItemString(dict, "windows", window_list);
  Py_DECREF(window_list);

  return AIS_OK;
}

// 6_1_15, 6_1_16, and 6_1_17 do not exist

// IMO Circ 289 - Clearance time to enter port
AIS_STATUS
ais6_1_18_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(pad < 6);
  Ais6_1_18 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "link_id", msg.link_id);
  DictSafeSetItem(dict, "utc_month", msg.utc_month);
  DictSafeSetItem(dict, "utc_day", msg.utc_day);
  DictSafeSetItem(dict, "utc_hour", msg.utc_hour);
  DictSafeSetItem(dict, "utc_min", msg.utc_min);
  DictSafeSetItem(dict, "port_berth", msg.port_berth);
  DictSafeSetItem(dict, "dest", msg.dest);
  DictSafeSetItem(dict, "x", "y", msg.position);
  DictSafeSetItem(dict, "spare2_0", msg.spare2[0]);
  DictSafeSetItem(dict, "spare2_1", msg.spare2[1]);

  return AIS_OK;
}

// 6_1_19 does not exist

// IMO Circ 289 - Berthing data
AIS_STATUS
ais6_1_20_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(pad < 6);
  Ais6_1_20 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "link_id", msg.link_id);
  DictSafeSetItem(dict, "length", msg.length);
  DictSafeSetItem(dict, "depth", msg.depth);
  DictSafeSetItem(dict, "position", msg.mooring_position);
  DictSafeSetItem(dict, "utc_month", msg.utc_month);
  DictSafeSetItem(dict, "utc_day", msg.utc_day);
  DictSafeSetItem(dict, "utc_hour", msg.utc_hour);
  DictSafeSetItem(dict, "utc_min", msg.utc_min);
  if (msg.services_known) {
    PyObject *serv_list = PyList_New(26);
    for (size_t serv_num = 0; serv_num < 26; serv_num++) {
      PyObject *serv = PyLong_FromLong(long(msg.services[serv_num]));  // NOLINT
      PyList_SetItem(serv_list, serv_num, serv);
    }
    DictSafeSetItem(dict, "services", serv_list);
    Py_DECREF(serv_list);
  }
  DictSafeSetItem(dict, "name", msg.name);
  DictSafeSetItem(dict, "x", "y", msg.position);

  return AIS_OK;
}

// 6_1_21, 6_1_22, 6_1_23 and 6_1_24 Do not exist (yet?)

AIS_STATUS
ais6_1_25_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais6_1_25 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "amount_unit", msg.amount_unit);
  DictSafeSetItem(dict, "amount", msg.amount);

  if (0 == msg.cargos.size())
    return AIS_ERR_BAD_SUB_MSG;

  PyObject *cargo_list = PyList_New(msg.cargos.size());
  for (size_t cargo_num = 0; cargo_num < msg.cargos.size(); cargo_num++) {
    PyObject *cargo = PyDict_New();
    if (msg.cargos[cargo_num].imdg_valid)
      DictSafeSetItem(cargo, "imdg", msg.cargos[cargo_num].imdg);
    if (msg.cargos[cargo_num].spare_valid)
      DictSafeSetItem(cargo, "spare", msg.cargos[cargo_num].spare);
    if (msg.cargos[cargo_num].un_valid)
      DictSafeSetItem(cargo, "un", msg.cargos[cargo_num].un);
    if (msg.cargos[cargo_num].bc_valid)
      DictSafeSetItem(cargo, "bc", msg.cargos[cargo_num].bc);
    if (msg.cargos[cargo_num].marpol_oil_valid)
      DictSafeSetItem(cargo, "marpol_oil", msg.cargos[cargo_num].marpol_oil);
    if (msg.cargos[cargo_num].marpol_cat_valid)
      DictSafeSetItem(cargo, "marpol_cat", msg.cargos[cargo_num].marpol_cat);
    PyList_SetItem(cargo_list, cargo_num, cargo);
  }
  PyDict_SetItemString(dict, "cargos", cargo_list);
  Py_DECREF(cargo_list);

  return AIS_OK;
}

// 6_1_26
// 6_1_27
// 6_1_28 - TODO(schwehr): Route Addressed - clone from 8_1_27
// 6_1_29
// 6_1_30 - TODO(schwehr): Text Addressed
// 6_1_31

AIS_STATUS
ais6_1_32_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais6_1_32 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "utc_month", msg.utc_month);
  DictSafeSetItem(dict, "utc_day", msg.utc_day);

  PyObject *window_list = PyList_New(msg.windows.size());
  for (size_t win_num = 0; win_num < msg.windows.size(); win_num++) {
    PyObject *win = PyDict_New();
    DictSafeSetItem(win, "x", "y", msg.windows[win_num].position);
    DictSafeSetItem(win, "from_utc_hour", msg.windows[win_num].from_utc_hour);
    DictSafeSetItem(win, "from_utc_min", msg.windows[win_num].from_utc_min);
    DictSafeSetItem(win, "to_utc_hour", msg.windows[win_num].to_utc_hour);
    DictSafeSetItem(win, "to_utc_min", msg.windows[win_num].to_utc_min);
    DictSafeSetItem(win, "cur_dir", msg.windows[win_num].cur_dir);
    DictSafeSetItem(win, "cur_speed", msg.windows[win_num].cur_speed);
    PyList_SetItem(window_list, win_num, win);
  }
  PyDict_SetItemString(dict, "windows", window_list);
  Py_DECREF(window_list);

  return AIS_OK;
}

AIS_STATUS
ais6_1_40_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais6_1_40 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }
  DictSafeSetItem(dict, "persons", msg.persons);
  DictSafeSetItem(dict, "spare2", msg.spare2);

  return AIS_OK;
}

PyObject*
ais6_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  Ais6 msg(nmea_payload, pad);
  if (msg.had_error() && msg.get_error() != AIS_UNINITIALIZED) {
    PyErr_Format(ais_py_exception, "Ais6: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  DictSafeSetItem(dict, "seq", msg.seq);
  DictSafeSetItem(dict, "mmsi_dest", msg.mmsi_dest);
  DictSafeSetItem(dict, "retransmit", msg.retransmit);
  DictSafeSetItem(dict, "spare", msg.spare);
  DictSafeSetItem(dict, "dac", msg.dac);
  DictSafeSetItem(dict, "fi", msg.fi);

  // TODO(schwehr): manage all the submessage types

  AIS_STATUS status = AIS_UNINITIALIZED;

  switch (msg.dac) {
  case AIS_DAC_1_INTERNATIONAL:  // IMO.
    switch (msg.fi) {
    case AIS_FI_6_1_0_TEXT:  // OLD ITU 1371-1.
      status = ais6_1_0_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_6_1_1_ACK:  // OLD ITU 1371-1.
      status = ais6_1_1_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_6_1_2_FI_INTERROGATE:  // OLD ITU 1371-1.
      status = ais6_1_2_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_6_1_3_CAPABILITY_INTERROGATE:  // OLD ITU 1371-1.
      status = ais6_1_3_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_6_1_4_CAPABILITY_REPLY:  // OLD ITU 1371-1.
      status = ais6_1_4_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_6_1_5_ACK:  // ITU 1371-5.
      status = ais6_1_5_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_6_1_12_DANGEROUS_CARGO:  // Not to be used after 1 Jan 2013.
      status = ais6_1_12_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_6_1_14_TIDAL_WINDOW:  // Not to be used after 1 Jan 2013.
      status = ais6_1_14_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_6_1_18_ENTRY_TIME:
      status = ais6_1_18_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_6_1_20_BERTHING:
      status = ais6_1_20_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_6_1_25_DANGEROUS_CARGO:
      status = ais6_1_25_append_pydict(nmea_payload, dict, pad);
      break;
      // TODO(schwehr): AIS_FI_6_1_28_ROUTE.
      // TODO(schwehr): AIS_FI_6_1_30_TEXT.
    case AIS_FI_6_1_32_TIDAL_WINDOW:  // IMO Circ 289
      status = ais6_1_32_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_6_1_40_PERSONS_ON_BOARD:  // OLD ITU 1371-1.
      status = ais6_1_40_append_pydict(nmea_payload, dict, pad);
      break;
    default:
      // TODO(schwehr): Raise an exception?
      DictSafeSetItem(dict, "not_parsed", true);
    }
    break;

    default:
      // TODO(schwehr): Raise an exception?
      DictSafeSetItem(dict, "not_parsed", true);
  }

  if (status != AIS_OK) {
    Py_DECREF(dict);
    PyErr_Format(ais_py_exception,
                 "Ais6: DAC:FI not known.  6:%d:%d %s",
                 msg.dac,
                 msg.fi,
                 AIS_STATUS_STRINGS[status]);
    return nullptr;
  }

  return dict;
}

// Acknowledgement
PyObject*
ais7_13_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  const Ais7_13 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais7_13: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  PyObject *list = PyList_New(msg.dest_mmsi.size());
  for (size_t i = 0; i < msg.dest_mmsi.size(); i++) {
    PyObject *tuple = PyTuple_New(2);
    PyTuple_SetItem(tuple, 0, PyLong_FromLong(msg.dest_mmsi[i]));  // Steals ref
    PyTuple_SetItem(tuple, 1, PyLong_FromLong(msg.seq_num[i]));  // Steals ref
    PyList_SetItem(list, i, tuple);  // Steals ref
  }
  PyDict_SetItemString(dict, "acks", list);
  Py_DECREF(list);
  return dict;
}

AIS_STATUS
ais8_1_0_append_pydict(const char *nmea_payload, PyObject *dict,
                       const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_1_0 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "ack_required",  msg.ack_required);
  DictSafeSetItem(dict, "msg_seq",  msg.msg_seq);
  DictSafeSetItem(dict, "text", msg.text);
  DictSafeSetItem(dict, "spare2", msg.spare2);

  return AIS_OK;
}

// ais 8_1_[1..10] do not exist

AIS_STATUS
ais8_1_11_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_1_11 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "x", "y", msg.position);

  DictSafeSetItem(dict, "wind_ave", msg.wind_ave);
  DictSafeSetItem(dict, "wind_gust", msg.wind_gust);
  DictSafeSetItem(dict, "wind_dir", msg.wind_dir);
  DictSafeSetItem(dict, "wind_gust_dir", msg.wind_gust);

  DictSafeSetItem(dict, "air_temp", msg.air_temp);
  DictSafeSetItem(dict, "rel_humid", msg.rel_humid);
  DictSafeSetItem(dict, "dew_point", msg.dew_point);
  DictSafeSetItem(dict, "air_pres", msg.air_pres);
  DictSafeSetItem(dict, "air_pres_trend", msg.air_pres_trend);
  DictSafeSetItem(dict, "horz_vis", msg.horz_vis);

  DictSafeSetItem(dict, "water_level", msg.water_level);
  DictSafeSetItem(dict, "water_level_trend", msg.water_level_trend);

  DictSafeSetItem(dict, "surf_cur_speed", msg.surf_cur_speed);
  DictSafeSetItem(dict, "surf_cur_dir", msg.surf_cur_dir);

  DictSafeSetItem(dict, "cur_speed_2", msg.cur_speed_2);
  DictSafeSetItem(dict, "cur_dir_2",   msg.cur_dir_2);
  DictSafeSetItem(dict, "cur_depth_2", msg.cur_depth_2);

  DictSafeSetItem(dict, "cur_speed_3", msg.cur_speed_3);
  DictSafeSetItem(dict, "cur_dir_3",   msg.cur_dir_3);
  DictSafeSetItem(dict, "cur_depth_3", msg.cur_depth_3);

  DictSafeSetItem(dict, "wave_height", msg.wave_height);
  DictSafeSetItem(dict, "wave_period", msg.wave_period);
  DictSafeSetItem(dict, "wave_dir", msg.wave_dir);

  DictSafeSetItem(dict, "swell_height", msg.swell_height);
  DictSafeSetItem(dict, "swell_period", msg.swell_period);
  DictSafeSetItem(dict, "swell_dir", msg.swell_dir);

  DictSafeSetItem(dict, "sea_state", msg.sea_state);
  DictSafeSetItem(dict, "water_temp", msg.water_temp);
  DictSafeSetItem(dict, "precip_type", msg.precip_type);
  DictSafeSetItem(dict, "ice", msg.ice);  // Grr... ice

  // Or could be spare
  DictSafeSetItem(dict, "ext_water_level", msg.extended_water_level);
  DictSafeSetItem(dict, "spare2", msg.extended_water_level);

  return AIS_OK;
}

// 12 is addressed

AIS_STATUS
ais8_1_13_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_1_13 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "reason", msg.reason);
  DictSafeSetItem(dict, "location_from", msg.location_from);
  DictSafeSetItem(dict, "location_to", msg.location_to);
  DictSafeSetItem(dict, "radius", msg.radius);
  DictSafeSetItem(dict, "units", msg.units);
  DictSafeSetItem(dict, "day_from", msg.day_from);
  DictSafeSetItem(dict, "month_from", msg.month_from);
  DictSafeSetItem(dict, "hour_from", msg.hour_from);
  DictSafeSetItem(dict, "minute_from", msg.minute_from);
  DictSafeSetItem(dict, "day_to", msg.day_to);
  DictSafeSetItem(dict, "month_to", msg.month_to);
  DictSafeSetItem(dict, "hour_to", msg.hour_to);
  DictSafeSetItem(dict, "minute_to", msg.minute_to);
  DictSafeSetItem(dict, "spare2", msg.spare2);

  return AIS_OK;
}

// 14 is addressed

AIS_STATUS
ais8_1_15_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_1_15 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "air_draught", msg.air_draught);
  DictSafeSetItem(dict, "spare2", msg.spare2);
  return AIS_OK;
}


AIS_STATUS
ais8_1_16_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_1_16 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "persons", msg.persons);
  DictSafeSetItem(dict, "spare2", msg.spare2);
  return AIS_OK;
}


AIS_STATUS
ais8_1_17_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  const Ais8_1_17 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  PyObject *target_list = PyList_New(msg.targets.size());
  for (size_t target_num = 0; target_num < msg.targets.size(); target_num++) {
    PyObject *target = PyDict_New();
    DictSafeSetItem(target, "type", msg.targets[target_num].type);
    DictSafeSetItem(target, "id", msg.targets[target_num].id);
    DictSafeSetItem(target, "spare", msg.targets[target_num].spare);
    DictSafeSetItem(target, "x", "y", msg.targets[target_num].position);
    DictSafeSetItem(target, "cog", msg.targets[target_num].cog);
    DictSafeSetItem(target, "timestamp", msg.targets[target_num].timestamp);
    DictSafeSetItem(target, "sog", msg.targets[target_num].sog);
    PyList_SetItem(target_list, target_num, target);
  }
  PyDict_SetItemString(dict, "targets", target_list);
  Py_DECREF(target_list);

  return AIS_OK;
}

// 18 is addressed only

AIS_STATUS
ais8_1_19_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_1_19 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "link_id", msg.link_id);
  DictSafeSetItem(dict, "name", msg.name);
  DictSafeSetItem(dict, "x", "y", msg.position);
  DictSafeSetItem(dict, "status", msg.status);
  DictSafeSetItem(dict, "signal", msg.signal);
  DictSafeSetItem(dict, "utc_hour_next", msg.utc_hour_next);
  DictSafeSetItem(dict, "utc_min_next", msg.utc_min_next);
  DictSafeSetItem(dict, "next_signal", msg.next_signal);
  DictSafeSetItem(dict, "spare2_0", msg.spare2[0]);
  DictSafeSetItem(dict, "spare2_1", msg.spare2[1]);
  DictSafeSetItem(dict, "spare2_2", msg.spare2[2]);
  DictSafeSetItem(dict, "spare2_3", msg.spare2[3]);

  return AIS_OK;
}

// 20 is addressed

AIS_STATUS
ais8_1_21_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_1_21 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "x", "y", msg.position);
  DictSafeSetItem(dict, "utc_day", msg.utc_day);
  DictSafeSetItem(dict, "utc_hour", msg.utc_hour);
  DictSafeSetItem(dict, "utc_min", msg.utc_min);

  if (0 == msg.type_wx_report) {
    // WX obs from ship
    DictSafeSetItem(dict, "location", msg.location);
    DictSafeSetItem(dict, "wx", msg.wx[0]);  // TODO(schwehr) Rename present?
    DictSafeSetItem(dict, "horz_viz", msg.horz_viz);
    DictSafeSetItem(dict, "humidity", msg.humidity);
    DictSafeSetItem(dict, "wind_speed", msg.wind_speed);
    DictSafeSetItem(dict, "wind_dir", msg.wind_dir);
    DictSafeSetItem(dict, "pressure", msg.pressure);
    DictSafeSetItem(dict, "pressure_tendency", msg.pressure_tendency);
    DictSafeSetItem(dict, "air_temp", msg.air_temp);
    DictSafeSetItem(dict, "water_temp", msg.water_temp);
    DictSafeSetItem(dict, "wave_period", msg.wave_period);
    DictSafeSetItem(dict, "wave_height", msg.wave_height);
    DictSafeSetItem(dict, "wave_dir", msg.wave_dir);
    DictSafeSetItem(dict, "swell_height", msg.swell_height);
    DictSafeSetItem(dict, "swell_dir", msg.swell_dir);
    DictSafeSetItem(dict, "swell_period", msg.swell_period);
    DictSafeSetItem(dict, "spare2", msg.spare2);
  } else {
    // type == 1
    // WMO OBS from ship
    DictSafeSetItem(dict, "utc_month", msg.utc_month);
    DictSafeSetItem(dict, "cog", msg.cog);
    DictSafeSetItem(dict, "sog", msg.sog);
    DictSafeSetItem(dict, "heading", msg.heading);
    DictSafeSetItem(dict, "pressure", msg.pressure);
    DictSafeSetItem(dict, "rel_pressure", msg.rel_pressure);
    DictSafeSetItem(dict, "pressure_tendency", msg.pressure_tendency);
    DictSafeSetItem(dict, "wind_dir", msg.wind_dir);
    DictSafeSetItem(dict, "wind_speed_ms", msg.wind_speed_ms);
    DictSafeSetItem(dict, "wind_dir_rel", msg.wind_dir_rel);
    DictSafeSetItem(dict, "wind_speed_rel", msg.wind_speed_rel);
    DictSafeSetItem(dict, "wind_gust_speed", msg.wind_gust_speed);
    DictSafeSetItem(dict, "wind_gust_dir", msg.wind_gust_dir);
    DictSafeSetItem(dict, "air_temp_raw", msg.air_temp_raw);
    DictSafeSetItem(dict, "humidity", msg.humidity);
    DictSafeSetItem(dict, "water_temp_raw", msg.water_temp_raw);
    DictSafeSetItem(dict, "horz_viz", msg.horz_viz);
    // TODO(schwehr): list?
    DictSafeSetItem(dict, "wx", msg.wx[0]);
    DictSafeSetItem(dict, "wx_next1", msg.wx[1]);
    DictSafeSetItem(dict, "wx_next2", msg.wx[2]);
    DictSafeSetItem(dict, "cloud_total", msg.cloud_total);
    DictSafeSetItem(dict, "cloud_low", msg.cloud_low);
    DictSafeSetItem(dict, "cloud_low_type", msg.cloud_low_type);
    DictSafeSetItem(dict, "cloud_middle_type", msg.cloud_middle_type);
    DictSafeSetItem(dict, "cloud_high_type", msg.cloud_high_type);
    DictSafeSetItem(dict, "alt_lowest_cloud_base", msg.alt_lowest_cloud_base);
    DictSafeSetItem(dict, "wave_period", msg.wave_period);
    DictSafeSetItem(dict, "wave_height", msg.wave_height);
    DictSafeSetItem(dict, "swell_dir", msg.swell_dir);
    DictSafeSetItem(dict, "swell_period", msg.swell_period);
    DictSafeSetItem(dict, "swell_height", msg.swell_height);
    DictSafeSetItem(dict, "swell_dir_2", msg.swell_dir_2);
    DictSafeSetItem(dict, "swell_period_2", msg.swell_period_2);
    DictSafeSetItem(dict, "swell_height_2", msg.swell_height_2);
    DictSafeSetItem(dict, "ice_thickness", msg.ice_thickness);
    DictSafeSetItem(dict, "ice_accretion", msg.ice_accretion);
    DictSafeSetItem(dict, "ice_accretion_cause", msg.ice_accretion_cause);
    DictSafeSetItem(dict, "sea_ice_concentration", msg.sea_ice_concentration);
    DictSafeSetItem(dict, "amt_type_ice", msg.amt_type_ice);
    DictSafeSetItem(dict, "ice_situation", msg.ice_situation);
    DictSafeSetItem(dict, "ice_devel", msg.ice_devel);
    DictSafeSetItem(dict, "bearing_ice_edge", msg.bearing_ice_edge);
  }

  return AIS_OK;
}


AIS_STATUS
ais8_1_22_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  Ais8_1_22 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "link_id", msg.link_id);
  DictSafeSetItem(dict, "notice_type", msg.notice_type);
  DictSafeSetItem(dict, "notice_type_str",
                  ais8_1_22_notice_names[msg.notice_type]);

  DictSafeSetItem(dict, "month", msg.month);
  DictSafeSetItem(dict, "day", msg.day);
  DictSafeSetItem(dict, "hour", msg.hour);
  DictSafeSetItem(dict, "minute", msg.minute);
  DictSafeSetItem(dict, "duration_minutes", msg.minute);

  PyObject *sub_area_list = PyList_New(msg.sub_areas.size());

  // Loop over sub_areas
  for (size_t i = 0; i < msg.sub_areas.size(); i++) {
    switch (msg.sub_areas[i]->getType()) {
    case AIS8_1_22_SHAPE_CIRCLE:  // or point
      {
        PyObject *sub_area = PyDict_New();
        Ais8_1_22_Circle *c =
            dynamic_cast<Ais8_1_22_Circle*>(msg.sub_areas[i].get());
        assert(c != nullptr);

        DictSafeSetItem(sub_area, "sub_area_type", AIS8_1_22_SHAPE_CIRCLE);
        if (c->radius_m == 0)
          DictSafeSetItem(sub_area, "sub_area_type_str", "point");
        else
          DictSafeSetItem(sub_area, "sub_area_type_str", "circle");

        DictSafeSetItem(sub_area, "x", "y", c->position);
        DictSafeSetItem(sub_area, "precision", c->precision);
        DictSafeSetItem(sub_area, "radius", c->radius_m);
        // TODO(schwehr): spare?
        PyList_SetItem(sub_area_list, i, sub_area);
      }
      break;
    case AIS8_1_22_SHAPE_RECT:
      {
        PyObject *sub_area = PyDict_New();
        Ais8_1_22_Rect *c =
            dynamic_cast<Ais8_1_22_Rect*>(msg.sub_areas[i].get());
        assert(c != nullptr);

        DictSafeSetItem(sub_area, "sub_area_type", AIS8_1_22_SHAPE_RECT);
        DictSafeSetItem(sub_area, "sub_area_type_str", "rect");

        DictSafeSetItem(sub_area, "x", "y", c->position);
        DictSafeSetItem(sub_area, "precision", c->precision);
        DictSafeSetItem(sub_area, "e_dim_m", c->e_dim_m);
        DictSafeSetItem(sub_area, "n_dim_m", c->n_dim_m);
        DictSafeSetItem(sub_area, "orient_deg", c->orient_deg);
        // TODO(schwehr): spare?
        PyList_SetItem(sub_area_list, i, sub_area);
      }
      break;
    case AIS8_1_22_SHAPE_SECTOR:
      {
        PyObject *sub_area = PyDict_New();
        Ais8_1_22_Sector *c =
            dynamic_cast<Ais8_1_22_Sector*>(msg.sub_areas[i].get());
        assert(c != nullptr);

        DictSafeSetItem(sub_area, "sub_area_type", AIS8_1_22_SHAPE_SECTOR);
        DictSafeSetItem(sub_area, "sub_area_type_str", "sector");

        DictSafeSetItem(sub_area, "x", "y", c->position);
        DictSafeSetItem(sub_area, "precision", c->precision);
        DictSafeSetItem(sub_area, "radius", c->radius_m);
        DictSafeSetItem(sub_area, "left_bound_deg", c->left_bound_deg);
        DictSafeSetItem(sub_area, "right_bound_deg", c->right_bound_deg);
        // TODO(schwehr): spare?
        PyList_SetItem(sub_area_list, i, sub_area);
      }
      break;
    case AIS8_1_22_SHAPE_POLYLINE:
      {
        PyObject *sub_area = PyDict_New();
        Ais8_1_22_Polyline *polyline =
            dynamic_cast<Ais8_1_22_Polyline*>(msg.sub_areas[i].get());
        assert(polyline != nullptr);

        DictSafeSetItem(sub_area, "sub_area_type", AIS8_1_22_SHAPE_POLYLINE);
        DictSafeSetItem(sub_area, "sub_area_type_str", "polyline");
        assert(polyline->angles.size() == polyline->dists_m.size());
        PyObject *angle_list = PyList_New(polyline->angles.size());
        PyObject *dist_list = PyList_New(polyline->angles.size());

        for (size_t pt_num = 0; pt_num < polyline->angles.size(); pt_num++) {
          PyList_SetItem(angle_list, pt_num,
                         PyFloat_FromDouble(polyline->angles[pt_num]));
          PyList_SetItem(dist_list, pt_num,
                         PyFloat_FromDouble(polyline->dists_m[pt_num]));
        }

        DictSafeSetItem(sub_area, "angles", angle_list);
        DictSafeSetItem(sub_area, "dists_m", dist_list);
        Py_DECREF(angle_list);
        Py_DECREF(dist_list);

        // TODO(schwehr): spare?
        PyList_SetItem(sub_area_list, i, sub_area);
      }
      break;
    case AIS8_1_22_SHAPE_POLYGON:
      {
        PyObject *sub_area = PyDict_New();
        Ais8_1_22_Polygon *polygon =
            dynamic_cast<Ais8_1_22_Polygon*>(msg.sub_areas[i].get());
        assert(polygon != nullptr);

        DictSafeSetItem(sub_area, "sub_area_type", AIS8_1_22_SHAPE_POLYGON);
        DictSafeSetItem(sub_area, "sub_area_type_str", "polygon");
        assert(polygon->angles.size() == polygon->dists_m.size());
        PyObject *angle_list = PyList_New(polygon->angles.size());
        PyObject *dist_list = PyList_New(polygon->angles.size());

        for (size_t pt_num = 0; pt_num < polygon->angles.size(); pt_num++) {
          PyList_SetItem(angle_list, pt_num,
                         PyFloat_FromDouble(polygon->angles[pt_num]));
          PyList_SetItem(dist_list, pt_num,
                         PyFloat_FromDouble(polygon->dists_m[pt_num]));
        }

        DictSafeSetItem(sub_area, "angles", angle_list);
        DictSafeSetItem(sub_area, "dists_m", dist_list);
        Py_DECREF(angle_list);
        Py_DECREF(dist_list);

        // TODO(schwehr): spare?
        PyList_SetItem(sub_area_list, i, sub_area);
      }
      break;
    case AIS8_1_22_SHAPE_TEXT:
      {
        PyObject *sub_area = PyDict_New();

        Ais8_1_22_Text *text =
            dynamic_cast<Ais8_1_22_Text*>(msg.sub_areas[i].get());
        assert(text != nullptr);

        DictSafeSetItem(sub_area, "sub_area_type", AIS8_1_22_SHAPE_TEXT);
        DictSafeSetItem(sub_area, "sub_area_type_str", "text");

        DictSafeSetItem(sub_area, "text", text->text);
        PyList_SetItem(sub_area_list, i, sub_area);
      }
      break;

    default:
      {}  // TODO(schwehr): Mark an unknown subarea or raise an exception.
    }
  }
  DictSafeSetItem(dict, "sub_areas", sub_area_list);
  Py_DECREF(sub_area_list);

  return AIS_OK;
}


// No 23 broadcast

// IMO Circ 289 - Extended ship static and voyage-related
AIS_STATUS
ais8_1_24_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_1_24 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "link_id", msg.link_id);
  DictSafeSetItem(dict, "air_draught", msg.air_draught);
  DictSafeSetItem(dict, "last_port", msg.last_port);

  PyObject *port_list = PyList_New(2);
  PyList_SetItem(port_list, 0, PyUnicode_FromString(msg.next_ports[0].c_str()));
  PyList_SetItem(port_list, 1, PyUnicode_FromString(msg.next_ports[0].c_str()));

  PyObject *solas_list = PyList_New(26);
  for (size_t solas_num = 0; solas_num < 26; solas_num++) {
    PyObject *solas = PyLong_FromLong(msg.solas_status[solas_num]);
    PyList_SetItem(solas_list, solas_num, solas);
  }
  DictSafeSetItem(dict, "port_list", port_list);
  Py_DECREF(port_list);
  DictSafeSetItem(dict, "solas", solas_list);
  Py_DECREF(solas_list);
  DictSafeSetItem(dict, "ice_class", msg.ice_class);
  DictSafeSetItem(dict, "shaft_power", msg.shaft_power);
  DictSafeSetItem(dict, "vhf", msg.vhf);
  DictSafeSetItem(dict, "lloyds_ship_type", msg.lloyds_ship_type);
  DictSafeSetItem(dict, "gross_tonnage", msg.gross_tonnage);
  DictSafeSetItem(dict, "laden_ballast", msg.laden_ballast);
  DictSafeSetItem(dict, "heavy_oil", msg.heavy_oil);
  DictSafeSetItem(dict, "light_oil", msg.light_oil);
  DictSafeSetItem(dict, "diesel", msg.diesel);
  DictSafeSetItem(dict, "bunker_oil", msg.bunker_oil);
  DictSafeSetItem(dict, "persons", msg.persons);
  DictSafeSetItem(dict, "spare2", msg.spare2);

  return AIS_OK;
}


// No 25 broadcast

AIS_STATUS
ais8_1_26_append_pydict_sensor_hdr(PyObject *dict,
                                   Ais8_1_26_SensorReport* rpt) {
  assert(dict);
  assert(rpt);
  DictSafeSetItem(dict, "report_type", rpt->report_type);
  DictSafeSetItem(dict, "utc_day", rpt->utc_day);
  DictSafeSetItem(dict, "utc_hr", rpt->utc_hr);
  DictSafeSetItem(dict, "utc_min", rpt->utc_min);
  DictSafeSetItem(dict, "site_id", rpt->site_id);

  return AIS_OK;
}


// IMO Circ 289 - Environmental
AIS_STATUS
ais8_1_26_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  const Ais8_1_26 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  PyObject *rpt_list = PyList_New(msg.reports.size());
  DictSafeSetItem(dict, "reports", rpt_list);

  for (size_t rpt_num = 0; rpt_num < msg.reports.size(); rpt_num++) {
    PyObject *rpt_dict = PyDict_New();
    PyList_SetItem(rpt_list, rpt_num, rpt_dict);

    switch (msg.reports[rpt_num]->report_type) {
      // case AIS8_1_26_SENSOR_ERROR:
    case AIS8_1_26_SENSOR_LOCATION:
      {
        Ais8_1_26_Location *rpt =
            reinterpret_cast<Ais8_1_26_Location *>(msg.reports[rpt_num]);
        ais8_1_26_append_pydict_sensor_hdr(rpt_dict, rpt);
        DictSafeSetItem(rpt_dict, "x", "y", rpt->position);
        DictSafeSetItem(rpt_dict, "z", rpt->z);
        DictSafeSetItem(rpt_dict, "owner", rpt->owner);
        DictSafeSetItem(rpt_dict, "timeout", rpt->timeout);
        DictSafeSetItem(rpt_dict, "spare", rpt->spare);
      }
      break;
    case AIS8_1_26_SENSOR_STATION:
      {
        Ais8_1_26_Station *rpt =
            reinterpret_cast<Ais8_1_26_Station *>(msg.reports[rpt_num]);
        DictSafeSetItem(rpt_dict, "name", rpt->name);
        DictSafeSetItem(rpt_dict, "spare", rpt->spare);
      }
      break;
    case AIS8_1_26_SENSOR_WIND:
      {
        Ais8_1_26_Wind *rpt =
            reinterpret_cast<Ais8_1_26_Wind *>(msg.reports[rpt_num]);
        DictSafeSetItem(rpt_dict, "wind_speed", rpt->wind_speed);
        DictSafeSetItem(rpt_dict, "wind_gust", rpt->wind_gust);
        DictSafeSetItem(rpt_dict, "wind_dir", rpt->wind_dir);
        DictSafeSetItem(rpt_dict, "wind_gust_dir", rpt->wind_gust_dir);
        DictSafeSetItem(rpt_dict, "sensor_type", rpt->sensor_type);
        DictSafeSetItem(rpt_dict, "wind_forecast", rpt->wind_forecast);
        DictSafeSetItem(
            rpt_dict, "wind_gust_forecast", rpt->wind_gust_forecast);
        DictSafeSetItem(rpt_dict, "wind_dir_forecast", rpt->wind_dir_forecast);
        DictSafeSetItem(rpt_dict, "utc_day_forecast", rpt->utc_day_forecast);
        DictSafeSetItem(rpt_dict, "utc_hour_forecast", rpt->utc_hour_forecast);
        DictSafeSetItem(rpt_dict, "utc_min_forecast", rpt->utc_min_forecast);
        DictSafeSetItem(rpt_dict, "duration", rpt->duration);
        DictSafeSetItem(rpt_dict, "spare", rpt->spare);
      }
      break;
    case AIS8_1_26_SENSOR_WATER_LEVEL:
      {
        Ais8_1_26_WaterLevel *rpt =
            reinterpret_cast<Ais8_1_26_WaterLevel *>(msg.reports[rpt_num]);
        DictSafeSetItem(rpt_dict, "type", rpt->type);
        DictSafeSetItem(rpt_dict, "level", rpt->level);
        DictSafeSetItem(rpt_dict, "trend", rpt->trend);
        DictSafeSetItem(rpt_dict, "vdatum", rpt->vdatum);
        DictSafeSetItem(rpt_dict, "sensor_type", rpt->sensor_type);
        DictSafeSetItem(rpt_dict, "forecast_type", rpt->forecast_type);
        DictSafeSetItem(rpt_dict, "level_forecast", rpt->level_forecast);
        DictSafeSetItem(rpt_dict, "utc_day_forecast", rpt->utc_day_forecast);
        DictSafeSetItem(rpt_dict, "utc_hour_forecast", rpt->utc_hour_forecast);
        DictSafeSetItem(rpt_dict, "utc_min_forecast", rpt->utc_min_forecast);
        DictSafeSetItem(rpt_dict, "duration", rpt->duration);
        DictSafeSetItem(rpt_dict, "spare", rpt->spare);
      }
      break;
    case AIS8_1_26_SENSOR_CURR_2D:
      {
        Ais8_1_26_Curr2D *rpt =
            reinterpret_cast<Ais8_1_26_Curr2D *>(msg.reports[rpt_num]);
        DictSafeSetItem(rpt_dict, "type", rpt->type);
        DictSafeSetItem(rpt_dict, "spare", rpt->spare);

        PyObject *curr_list = PyList_New(3);
        DictSafeSetItem(dict, "currents", curr_list);
        for (size_t idx = 0; idx < 3; idx++) {
          PyObject *curr_dict = PyDict_New();
          DictSafeSetItem(curr_dict, "speed", rpt->currents[idx].speed);
          DictSafeSetItem(curr_dict, "dir", rpt->currents[idx].dir);
          DictSafeSetItem(curr_dict, "depth", rpt->currents[idx].depth);
          PyList_SetItem(curr_list, idx, curr_dict);
        }
        Py_DECREF(curr_list);
      }
      break;
    case AIS8_1_26_SENSOR_CURR_3D:
      {
        Ais8_1_26_Curr3D *rpt =
            reinterpret_cast<Ais8_1_26_Curr3D *>(msg.reports[rpt_num]);
        DictSafeSetItem(rpt_dict, "type", rpt->type);
        DictSafeSetItem(rpt_dict, "spare", rpt->spare);

        PyObject *curr_list = PyList_New(3);
        DictSafeSetItem(dict, "currents", curr_list);
        for (size_t idx = 0; idx < 2; idx++) {
          // ERROR: no way to specify negative direction
          PyObject *curr_dict = PyDict_New();
          PyList_SetItem(curr_list, idx, curr_dict);
          DictSafeSetItem(curr_dict, "north", rpt->currents[idx].north);
          DictSafeSetItem(curr_dict, "east", rpt->currents[idx].east);
          DictSafeSetItem(curr_dict, "up", rpt->currents[idx].up);
          DictSafeSetItem(curr_dict, "depth", rpt->currents[idx].depth);
        }
        Py_DECREF(curr_list);
      }
      break;
    case AIS8_1_26_SENSOR_HORZ_FLOW:
      {
        Ais8_1_26_HorzFlow *rpt =
            reinterpret_cast<Ais8_1_26_HorzFlow *>(msg.reports[rpt_num]);
        DictSafeSetItem(rpt_dict, "spare", rpt->spare);

        PyObject *curr_list = PyList_New(3);
        DictSafeSetItem(dict, "currents", curr_list);
        for (size_t idx = 0; idx < 2; idx++) {
          PyObject *curr_dict = PyDict_New();
          PyList_SetItem(curr_list, idx, curr_dict);
          DictSafeSetItem(curr_dict, "bearing", rpt->currents[idx].bearing);
          DictSafeSetItem(curr_dict, "dist", rpt->currents[idx].dist);
          DictSafeSetItem(curr_dict, "speed", rpt->currents[idx].speed);
          DictSafeSetItem(curr_dict, "dir", rpt->currents[idx].dir);
          DictSafeSetItem(curr_dict, "level", rpt->currents[idx].level);
        }
        Py_DECREF(curr_list);
      }
      break;
    case AIS8_1_26_SENSOR_SEA_STATE:
      {
        Ais8_1_26_SeaState *rpt =
            reinterpret_cast<Ais8_1_26_SeaState *>(msg.reports[rpt_num]);
        DictSafeSetItem(rpt_dict, "swell_height", rpt->swell_height);
        DictSafeSetItem(rpt_dict, "swell_period", rpt->swell_period);
        DictSafeSetItem(rpt_dict, "swell_dir", rpt->swell_dir);
        DictSafeSetItem(rpt_dict, "sea_state", rpt->sea_state);
        DictSafeSetItem(rpt_dict, "swell_sensor_type", rpt->swell_sensor_type);
        DictSafeSetItem(rpt_dict, "water_temp", rpt->water_temp);
        DictSafeSetItem(rpt_dict, "water_temp_depth", rpt->water_temp_depth);
        DictSafeSetItem(rpt_dict, "water_sensor_type", rpt->water_sensor_type);
        DictSafeSetItem(rpt_dict, "wave_height", rpt->wave_height);
        DictSafeSetItem(rpt_dict, "wave_period", rpt->wave_period);
        DictSafeSetItem(rpt_dict, "wave_dir", rpt->wave_dir);
        DictSafeSetItem(rpt_dict, "wave_sensor_type", rpt->wave_sensor_type);
        DictSafeSetItem(rpt_dict, "salinity", rpt->salinity);
      }
      break;
    case AIS8_1_26_SENSOR_SALINITY:
      {
        Ais8_1_26_Salinity *rpt =
            reinterpret_cast<Ais8_1_26_Salinity *>(msg.reports[rpt_num]);
        DictSafeSetItem(rpt_dict, "water_temp", rpt->water_temp);
        DictSafeSetItem(rpt_dict, "conductivity", rpt->conductivity);
        DictSafeSetItem(rpt_dict, "pressure", rpt->pressure);
        DictSafeSetItem(rpt_dict, "salinity", rpt->salinity);
        DictSafeSetItem(rpt_dict, "salinity_type", rpt->salinity_type);
        DictSafeSetItem(rpt_dict, "sensor_type", rpt->sensor_type);
        DictSafeSetItem(rpt_dict, "spare0", rpt->spare[0]);
        DictSafeSetItem(rpt_dict, "spare1", rpt->spare[1]);
      }
      break;
    case AIS8_1_26_SENSOR_WX:
      {
        Ais8_1_26_Wx *rpt =
            reinterpret_cast<Ais8_1_26_Wx *>(msg.reports[rpt_num]);
        DictSafeSetItem(rpt_dict, "air_temp", rpt->air_temp);
        DictSafeSetItem(rpt_dict, "air_temp_sensor_type",
                        rpt->air_temp_sensor_type);
        DictSafeSetItem(rpt_dict, "precip", rpt->precip);
        DictSafeSetItem(rpt_dict, "horz_vis", rpt->horz_vis);
        DictSafeSetItem(rpt_dict, "dew_point", rpt->dew_point);
        DictSafeSetItem(rpt_dict, "dew_point_type", rpt->dew_point_type);
        DictSafeSetItem(rpt_dict, "air_pressure", rpt->air_pressure);
        DictSafeSetItem(rpt_dict, "air_pressure_trend",
                        rpt->air_pressure_trend);
        DictSafeSetItem(rpt_dict, "air_pressor_type", rpt->air_pressor_type);
        DictSafeSetItem(rpt_dict, "salinity", rpt->salinity);
        DictSafeSetItem(rpt_dict, "spare", rpt->spare);
      }
      break;
    case AIS8_1_26_SENSOR_AIR_DRAUGHT:
      {
        Ais8_1_26_AirDraught *rpt =
            reinterpret_cast<Ais8_1_26_AirDraught *>(msg.reports[rpt_num]);
        DictSafeSetItem(rpt_dict, "draught", rpt->draught);
        DictSafeSetItem(rpt_dict, "gap", rpt->gap);
        DictSafeSetItem(rpt_dict, "forecast_gap", rpt->forecast_gap);
        DictSafeSetItem(rpt_dict, "int trend", rpt->trend);
        DictSafeSetItem(
            rpt_dict, "int utc_day_forecast", rpt->utc_day_forecast);
        DictSafeSetItem(rpt_dict, "utc_hour_forecast", rpt->utc_hour_forecast);
        DictSafeSetItem(rpt_dict, "utc_min_forecast", rpt->utc_min_forecast);
        DictSafeSetItem(rpt_dict, "spare", rpt->spare);
      }
      break;
    case AIS8_1_26_SENSOR_RESERVED_11:  // FALLTHROUGH
    case AIS8_1_26_SENSOR_RESERVED_12:
    case AIS8_1_26_SENSOR_RESERVED_13:
    case AIS8_1_26_SENSOR_RESERVED_14:
    case AIS8_1_26_SENSOR_RESERVED_15:
    default:
      {}  // TODO(schwehr): mark a bad sensor type or raise exception
    }
  }
  Py_DECREF(rpt_list);

  return AIS_OK;
}


// IMO Circ 289 - Route information
AIS_STATUS
ais8_1_27_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_1_27 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "link_id", msg.link_id);
  DictSafeSetItem(dict, "sender_type", msg.sender_type);
  DictSafeSetItem(dict, "route_type", msg.route_type);
  DictSafeSetItem(dict, "utc_month", msg.utc_month);
  DictSafeSetItem(dict, "utc_day", msg.utc_day);
  DictSafeSetItem(dict, "utc_hour", msg.utc_hour);
  DictSafeSetItem(dict, "utc_min", msg.utc_min);
  DictSafeSetItem(dict, "duration", msg.duration);

  PyObject *waypoint_list = PyList_New(msg.waypoints.size());
  for (size_t point_num = 0; point_num < msg.waypoints.size(); point_num++) {
    PyObject *waypoint = PyList_New(2);
    // TODO(schwehr): Py_DECREF(); ?
    PyList_SetItem(
        waypoint, 0, PyFloat_FromDouble(msg.waypoints[point_num].lng_deg));
    PyList_SetItem(
        waypoint, 1, PyFloat_FromDouble(msg.waypoints[point_num].lat_deg));
    PyList_SetItem(waypoint_list, point_num, waypoint);
  }
  PyDict_SetItemString(dict, "waypoints", waypoint_list);
  Py_DECREF(waypoint_list);

  return AIS_OK;
}


// no 28 broadcast

// IMO Circ 289 - Text description
AIS_STATUS
ais8_1_29_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_1_29 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "link_id", msg.link_id);
  DictSafeSetItem(dict, "text", msg.text);

  return AIS_OK;
}


// no 30 broadcast

AIS_STATUS
ais8_1_31_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_1_31 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "x", "y", msg.position);
  DictSafeSetItem(dict, "position_accuracy", msg.position_accuracy);
  DictSafeSetItem(dict, "utc_day", msg.utc_day);
  DictSafeSetItem(dict, "utc_hour", msg.utc_hour);
  DictSafeSetItem(dict, "utc_min", msg.utc_min);
  DictSafeSetItem(dict, "wind_ave", msg.wind_ave);
  DictSafeSetItem(dict, "wind_gust", msg.wind_gust);
  DictSafeSetItem(dict, "wind_dir", msg.wind_dir);
  DictSafeSetItem(dict, "wind_gust_dir", msg.wind_gust_dir);
  DictSafeSetItem(dict, "air_temp", msg.air_temp);
  DictSafeSetItem(dict, "rel_humid", msg.rel_humid);
  DictSafeSetItem(dict, "dew_point", msg.dew_point);
  DictSafeSetItem(dict, "air_pres", msg.air_pres);
  DictSafeSetItem(dict, "air_pres_trend", msg.air_pres_trend);
  DictSafeSetItem(dict, "horz_vis", msg.horz_vis);
  DictSafeSetItem(dict, "water_level", msg.water_level);
  DictSafeSetItem(dict, "water_level_trend", msg.water_level_trend);

  // TODO(schwehr): make this a list of dicts
  DictSafeSetItem(dict, "surf_cur_speed", msg.surf_cur_speed);
  DictSafeSetItem(dict, "surf_cur_dir", msg.surf_cur_dir);
  DictSafeSetItem(dict, "cur_speed_2", msg.cur_speed_2);
  DictSafeSetItem(dict, "cur_dir_2", msg.cur_dir_2);
  DictSafeSetItem(dict, "cur_depth_2", msg.cur_depth_2);
  DictSafeSetItem(dict, "cur_speed_3", msg.cur_speed_3);
  DictSafeSetItem(dict, "cur_dir_3", msg.cur_dir_3);
  DictSafeSetItem(dict, "cur_depth_3", msg.cur_depth_3);

  DictSafeSetItem(dict, "wave_height", msg.wave_height);
  DictSafeSetItem(dict, "wave_period", msg.wave_period);
  DictSafeSetItem(dict, "wave_dir", msg.wave_dir);
  DictSafeSetItem(dict, "swell_height", msg.swell_height);
  DictSafeSetItem(dict, "swell_period", msg.swell_period);
  DictSafeSetItem(dict, "swell_dir", msg.swell_dir);
  DictSafeSetItem(dict, "sea_state", msg.sea_state);
  DictSafeSetItem(dict, "water_temp", msg.water_temp);
  DictSafeSetItem(dict, "precip_type", msg.precip_type);
  DictSafeSetItem(dict, "salinity", msg.salinity);
  DictSafeSetItem(dict, "ice", msg.ice);

  return AIS_OK;
}


// no 32 broadcast

// DAC 200 - River Information System
// Inland ship static and voyage related data
AIS_STATUS
ais8_200_10_append_pydict(const char *nmea_payload, PyObject *dict,
                          const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_200_10 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "eu_id", msg.eu_id);
  DictSafeSetItem(dict, "length", msg.length);
  DictSafeSetItem(dict, "beam", msg.beam);
  DictSafeSetItem(dict, "ship_type", msg.ship_type);
  DictSafeSetItem(dict, "haz_cargo", msg.haz_cargo);
  DictSafeSetItem(dict, "draught", msg.draught);
  DictSafeSetItem(dict, "loaded", msg.loaded);
  DictSafeSetItem(dict, "speed_qual", msg.speed_qual);
  DictSafeSetItem(dict, "course_qual", msg.course_qual);
  DictSafeSetItem(dict, "heading_qual", msg.heading_qual);
  DictSafeSetItem(dict, "spare2", msg.spare2);

  return AIS_OK;
}

// River Information System
// ETA report
AIS_STATUS
ais8_200_21_append_pydict(const char *nmea_payload, PyObject *dict,
                          const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_200_21 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "country", msg.country);
  DictSafeSetItem(dict, "location", msg.location);
  DictSafeSetItem(dict, "section", msg.section);
  DictSafeSetItem(dict, "terminal", msg.terminal);
  DictSafeSetItem(dict, "hectometre", msg.hectometre);
  DictSafeSetItem(dict, "eta_month", msg.eta_month);
  DictSafeSetItem(dict, "eta_day", msg.eta_day);
  DictSafeSetItem(dict, "eta_hour", msg.eta_hour);
  DictSafeSetItem(dict, "eta_minute", msg.eta_minute);
  DictSafeSetItem(dict, "tugboats", msg.tugboats);
  DictSafeSetItem(dict, "air_draught", msg.air_draught);
  DictSafeSetItem(dict, "spare2", msg.spare2);

  return AIS_OK;
}

// River Information System
// RTA report
AIS_STATUS
ais8_200_22_append_pydict(const char *nmea_payload, PyObject *dict,
                          const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_200_22 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "country", msg.country);
  DictSafeSetItem(dict, "location", msg.location);
  DictSafeSetItem(dict, "section", msg.section);
  DictSafeSetItem(dict, "terminal", msg.terminal);
  DictSafeSetItem(dict, "hectometre", msg.hectometre);
  DictSafeSetItem(dict, "rta_month", msg.rta_month);
  DictSafeSetItem(dict, "rta_day", msg.rta_day);
  DictSafeSetItem(dict, "rta_hour", msg.rta_hour);
  DictSafeSetItem(dict, "rta_minute", msg.rta_minute);
  DictSafeSetItem(dict, "lock_status", msg.lock_status);
  DictSafeSetItem(dict, "spare2", msg.spare2);

  return AIS_OK;
}

// River Information System
// EMMA warning
AIS_STATUS
ais8_200_23_append_pydict(const char *nmea_payload, PyObject *dict,
                          const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_200_23 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "utc_year_start", msg.utc_year_start);
  DictSafeSetItem(dict, "utc_month_start", msg.utc_month_start);
  DictSafeSetItem(dict, "utc_day_start", msg.utc_day_start);
  DictSafeSetItem(dict, "utc_year_end", msg.utc_year_end);
  DictSafeSetItem(dict, "utc_month_end", msg.utc_month_end);
  DictSafeSetItem(dict, "utc_day_end", msg.utc_day_end);
  DictSafeSetItem(dict, "utc_hour_start", msg.utc_hour_start);
  DictSafeSetItem(dict, "utc_min_start", msg.utc_min_start);
  DictSafeSetItem(dict, "utc_hour_end", msg.utc_hour_end);
  DictSafeSetItem(dict, "utc_min_end", msg.utc_min_end);
  DictSafeSetItem(dict, "x1", "y1", msg.position1);
  DictSafeSetItem(dict, "x2", "y2", msg.position2);
  DictSafeSetItem(dict, "type", msg.type);
  DictSafeSetItem(dict, "min", msg.min);
  DictSafeSetItem(dict, "max", msg.max);
  DictSafeSetItem(dict, "classification", msg.classification);
  DictSafeSetItem(dict, "wind_dir", msg.wind_dir);
  DictSafeSetItem(dict, "spare2", msg.spare2);

  return AIS_OK;
}


// EU River Information System (RIS)
// Water level
AIS_STATUS
ais8_200_24_append_pydict(const char *nmea_payload, PyObject *dict,
                          const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  const Ais8_200_24 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "country", msg.country);

  PyObject *id_list = PyList_New(4);
  for (size_t i = 0; i < 4; i++)
    PyList_SetItem(id_list, i, PyLong_FromLong(msg.gauge_ids[i]));
  DictSafeSetItem(dict, "gauge_ids", id_list);
  Py_DECREF(id_list);

  PyObject *level_list = PyList_New(4);
  for (size_t i = 0; i < 4; i++)
    PyList_SetItem(level_list, i, PyFloat_FromDouble(msg.levels[i]));
  DictSafeSetItem(dict, "levels", level_list);
  Py_DECREF(level_list);

  return AIS_OK;
}


// River Information System
// Signal status
AIS_STATUS
ais8_200_40_append_pydict(const char *nmea_payload, PyObject *dict,
                          const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_200_40 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "x", "y", msg.position);
  DictSafeSetItem(dict, "form", msg.form);
  DictSafeSetItem(dict, "dir", msg.dir);
  DictSafeSetItem(dict, "stream_dir", msg.stream_dir);
  DictSafeSetItem(dict, "status_raw", msg.status_raw);
  // TODO(schwehr): extract status components
  DictSafeSetItem(dict, "spare2", msg.spare2);

  return AIS_OK;
}


// River Information System
// Number of persons on board
AIS_STATUS
ais8_200_55_append_pydict(const char *nmea_payload, PyObject *dict,
                          const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_200_55 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "crew", msg.crew);
  DictSafeSetItem(dict, "passengers", msg.passengers);
  DictSafeSetItem(dict, "yet_more_personnel", msg.yet_more_personnel);

  PyObject *spare2_list = PyList_New(3);
  for (size_t i = 0; i < 3; i++)
    PyList_SetItem(spare2_list, 0,  PyLong_FromLong(msg.spare2[i]));
  DictSafeSetItem(dict, "spare2", spare2_list);
  Py_DECREF(spare2_list);

  return AIS_OK;
}

void
ais8_367_22_append_pydict(const char *nmea_payload, PyObject *dict,
                          const size_t pad) {
  Ais8_367_22 msg(nmea_payload, pad);  // TODO(schwehr): check for errors

  DictSafeSetItem(dict, "version", msg.version);
  DictSafeSetItem(dict, "link_id", msg.link_id);
  DictSafeSetItem(dict, "notice_type", msg.notice_type);
  // TODO(schwehr): are 8:1:22 and 8:367:22 tables the same?
  DictSafeSetItem(dict, "notice_type_str",
                  ais8_1_22_notice_names[msg.notice_type]);

  DictSafeSetItem(dict, "month", msg.month);  // This is UTC, not local time.
  DictSafeSetItem(dict, "day", msg.day);
  DictSafeSetItem(dict, "hour", msg.hour);
  DictSafeSetItem(dict, "minute", msg.minute);

  DictSafeSetItem(dict, "durations_minutes", msg.duration_minutes);

  PyObject *sub_area_list = PyList_New(msg.sub_areas.size());

  // Loop over sub_areas
  for (size_t i = 0; i < msg.sub_areas.size(); i++) {
    switch (msg.sub_areas[i]->getType()) {
    case AIS8_366_22_SHAPE_CIRCLE:  // or point
      {
        PyObject *sub_area = PyDict_New();
        Ais8_367_22_Circle *c =
            dynamic_cast<Ais8_367_22_Circle*>(msg.sub_areas[i].get());
        assert(c != nullptr);

        DictSafeSetItem(sub_area, "sub_area_type", AIS8_366_22_SHAPE_CIRCLE);
        if (c->radius_m == 0)
          DictSafeSetItem(sub_area, "sub_area_type_str", "point");
        else
          DictSafeSetItem(sub_area, "sub_area_type_str", "circle");

        DictSafeSetItem(sub_area, "x", "y", c->position);
        DictSafeSetItem(sub_area, "precision", c->precision);
        DictSafeSetItem(sub_area, "radius", c->radius_m);
        PyList_SetItem(sub_area_list, i, sub_area);
      }
      break;
    case AIS8_366_22_SHAPE_RECT:
      {
        PyObject *sub_area = PyDict_New();
        Ais8_367_22_Rect *c =
            dynamic_cast<Ais8_367_22_Rect*>(msg.sub_areas[i].get());
        assert(c != nullptr);

        DictSafeSetItem(sub_area, "sub_area_type", AIS8_366_22_SHAPE_RECT);
        DictSafeSetItem(sub_area, "sub_area_type_str", "rect");

        DictSafeSetItem(sub_area, "x", "y", c->position);
        DictSafeSetItem(sub_area, "precision", c->precision);
        DictSafeSetItem(sub_area, "e_dim_m", c->e_dim_m);
        DictSafeSetItem(sub_area, "n_dim_m", c->n_dim_m);
        DictSafeSetItem(sub_area, "orient_deg", c->orient_deg);

        PyList_SetItem(sub_area_list, i, sub_area);
      }
      break;
    case AIS8_366_22_SHAPE_SECTOR:
      {
        PyObject *sub_area = PyDict_New();
        Ais8_367_22_Sector *c =
            dynamic_cast<Ais8_367_22_Sector*>(msg.sub_areas[i].get());
        assert(c != nullptr);

        DictSafeSetItem(sub_area, "sub_area_type", AIS8_366_22_SHAPE_SECTOR);
        DictSafeSetItem(sub_area, "sub_area_type_str", "sector");

        DictSafeSetItem(sub_area, "x", "y", c->position);
        DictSafeSetItem(sub_area, "precision", c->precision);
        DictSafeSetItem(sub_area, "radius", c->radius_m);
        DictSafeSetItem(sub_area, "left_bound_deg", c->left_bound_deg);
        DictSafeSetItem(sub_area, "right_bound_deg", c->right_bound_deg);

        PyList_SetItem(sub_area_list, i, sub_area);
      }
      break;
    case AIS8_366_22_SHAPE_POLYLINE:  // FALLTHROUGH
    case AIS8_366_22_SHAPE_POLYGON:
      {
        PyObject *sub_area = PyDict_New();
        Ais8_367_22_Poly *poly =
            dynamic_cast<Ais8_367_22_Poly*>(msg.sub_areas[i].get());
        assert(poly != nullptr);

        DictSafeSetItem(sub_area, "sub_area_type", msg.sub_areas[i]->getType());
        if (msg.sub_areas[i]->getType() == AIS8_366_22_SHAPE_POLYLINE)
          DictSafeSetItem(sub_area, "sub_area_type_str", "polyline");
        else
          DictSafeSetItem(sub_area, "sub_area_type_str", "polygon");
        assert(poly->angles.size() == poly->dists_m.size());
        PyObject *angle_list = PyList_New(poly->angles.size());
        PyObject *dist_list = PyList_New(poly->angles.size());

        for (size_t pt_num = 0; pt_num < poly->angles.size(); pt_num++) {
          PyList_SetItem(angle_list, pt_num,
                         PyFloat_FromDouble(poly->angles[pt_num]));
          PyList_SetItem(dist_list, pt_num,
                         PyFloat_FromDouble(poly->dists_m[pt_num]));
        }

        DictSafeSetItem(sub_area, "angles", angle_list);
        DictSafeSetItem(sub_area, "dists_m", dist_list);
        Py_DECREF(angle_list);
        Py_DECREF(dist_list);

        PyList_SetItem(sub_area_list, i, sub_area);
      }
      break;
    case AIS8_366_22_SHAPE_TEXT:
      {
        PyObject *sub_area = PyDict_New();

        Ais8_367_22_Text *text =
            dynamic_cast<Ais8_367_22_Text*>(msg.sub_areas[i].get());
        assert(text != nullptr);

        DictSafeSetItem(sub_area, "sub_area_type", AIS8_366_22_SHAPE_TEXT);
        DictSafeSetItem(sub_area, "sub_area_type_str", "text");

        DictSafeSetItem(sub_area, "text", text->text);

        PyList_SetItem(sub_area_list, i, sub_area);
      }
      break;

    default:
      {}  // TODO(schwehr): Mark an unknown subarea or raise an exception.
    }
  }
  DictSafeSetItem(dict, "sub_areas", sub_area_list);
  Py_DECREF(sub_area_list);
}

AIS_STATUS
ais8_367_23_append_pydict(const char *nmea_payload, PyObject *dict,
                          const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_367_23 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "version", msg.version);

  if (msg.utc_day != 0) {
    DictSafeSetItem(dict, "utc_day", msg.utc_day);
  } else {
    DictSafeSetItem(dict, "utc_day", Py_None);
  }

  if (msg.utc_hour <= 23) {
    DictSafeSetItem(dict, "utc_hour", msg.utc_hour);
  } else {
    DictSafeSetItem(dict, "utc_hour", Py_None);
  }

  if (msg.utc_min <= 59) {
    DictSafeSetItem(dict, "utc_min", msg.utc_min);
  } else {
    DictSafeSetItem(dict, "utc_min", Py_None);
  }

  DictSafeSetItem(dict, "x", "y", msg.position);

  if (msg.pressure <= (402 + 799)) {
    DictSafeSetItem(dict, "pressure", msg.pressure);
  } else {
    // Raw value was 403 (N/A), or reserved value.
    DictSafeSetItem(dict, "pressure", Py_None);
  }

  if (msg.air_temp_raw >= -600 && msg.air_temp_raw <= 600) {
    DictSafeSetItem(dict, "air_temp", msg.air_temp);
  } else {
    DictSafeSetItem(dict, "air_temp", Py_None);
  }

  if (msg.wind_speed <= 121) {
    DictSafeSetItem(dict, "wind_speed", msg.wind_speed);
  } else {
    // Raw value was 122 (N/A) or reserved value.
    DictSafeSetItem(dict, "wind_speed", Py_None);
  }

  if (msg.wind_gust <= 121) {
    DictSafeSetItem(dict, "wind_gust", msg.wind_gust);
  } else {
    // Raw value was 122 (N/A) or reserved value.
    DictSafeSetItem(dict, "wind_gust", Py_None);
  }

  if (msg.wind_dir <= 359) {
    DictSafeSetItem(dict, "wind_dir", msg.wind_dir);
  } else {
    // Raw value was 360 (N/A), or reserved value.
    DictSafeSetItem(dict, "wind_dir", Py_None);
  }

  return AIS_OK;
}

AIS_STATUS
ais8_367_24_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_367_24 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "version", msg.version);

  if (msg.utc_hour <= 23) {
    DictSafeSetItem(dict, "utc_hour", msg.utc_hour);
  } else {
    DictSafeSetItem(dict, "utc_hour", Py_None);
  }

  if (msg.utc_min <= 59) {
    DictSafeSetItem(dict, "utc_min", msg.utc_min);
  } else {
    DictSafeSetItem(dict, "utc_min", Py_None);
  }

  DictSafeSetItem(dict, "x", "y", msg.position);

  if (msg.pressure <= (403 + 799)) {
    DictSafeSetItem(dict, "pressure", msg.pressure);
  } else {
    // Raw value was 403 (N/A), or reserved value.
    DictSafeSetItem(dict, "pressure", Py_None);
  }

  return AIS_OK;
}

AIS_STATUS
ais8_367_25_append_pydict(const char *nmea_payload, PyObject *dict,
                        const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  Ais8_367_25 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  DictSafeSetItem(dict, "version", msg.version);

  if (msg.utc_hour <= 23) {
    DictSafeSetItem(dict, "utc_hour", msg.utc_hour);
  } else {
    DictSafeSetItem(dict, "utc_hour", Py_None);
  }

  if (msg.utc_min <= 59) {
    DictSafeSetItem(dict, "utc_min", msg.utc_min);
  } else {
    DictSafeSetItem(dict, "utc_min", Py_None);
  }

  if (msg.pressure <= (799 + 402)) {
    DictSafeSetItem(dict, "pressure", msg.pressure);
  } else {
    // Raw value was 403 (N/A), or reserved value.
    DictSafeSetItem(dict, "pressure", Py_None);
  }

  if (msg.wind_speed <= 121) {
    DictSafeSetItem(dict, "wind_speed", msg.wind_speed);
  } else {
    // Raw value was 122 (N/A) or reserved value.
    DictSafeSetItem(dict, "wind_speed", Py_None);
  }

  if (msg.wind_dir <= 359) {
    DictSafeSetItem(dict, "wind_dir", msg.wind_dir);
  } else {
    // Raw value was 360 (N/A), or reserved value.
    DictSafeSetItem(dict, "wind_dir", Py_None);
  }

  return AIS_OK;
}

AIS_STATUS
ais8_367_33_append_pydict_sensor_hdr(PyObject *dict,
                                     Ais8_367_33_SensorReport* rpt) {
  assert(dict);
  assert(rpt);
  DictSafeSetItem(dict, "report_type", rpt->report_type);

  if (rpt->utc_day != 0) {
    DictSafeSetItem(dict, "utc_day", rpt->utc_day);
  } else {
    DictSafeSetItem(dict, "utc_day", Py_None);
  }

  if (rpt->utc_hr <= 23) {
    DictSafeSetItem(dict, "utc_hr", rpt->utc_hr);
  } else {
    DictSafeSetItem(dict, "utc_hr", Py_None);
  }

  if (rpt->utc_min <= 59) {
    DictSafeSetItem(dict, "utc_min", rpt->utc_min);
  } else {
    DictSafeSetItem(dict, "utc_min", Py_None);
  }

  DictSafeSetItem(dict, "site_id", rpt->site_id);

  return AIS_OK;
}

AIS_STATUS
ais8_367_33_append_pydict(const char *nmea_payload, PyObject *dict,
                          const size_t pad) {
  assert(nmea_payload);
  assert(dict);
  assert(pad < 6);
  const Ais8_367_33 msg(nmea_payload, pad);
  if (msg.had_error()) {
    return msg.get_error();
  }

  PyObject *rpt_list = PyList_New(msg.reports.size());
  DictSafeSetItem(dict, "reports", rpt_list);

  for (size_t rpt_num = 0; rpt_num < msg.reports.size(); rpt_num++) {
    PyObject *rpt_dict = PyDict_New();
    PyList_SetItem(rpt_list, rpt_num, rpt_dict);

    switch (msg.reports[rpt_num]->report_type) {
      // case AIS8_367_33_SENSOR_ERROR:
    case AIS8_367_33_SENSOR_LOCATION:
      {
        Ais8_367_33_Location *rpt =
            dynamic_cast<Ais8_367_33_Location *>(msg.reports[rpt_num].get());
        assert(rpt != nullptr);
        ais8_367_33_append_pydict_sensor_hdr(rpt_dict, rpt);

        DictSafeSetItem(rpt_dict, "x", "y", rpt->position);

        if (rpt->altitude_raw >= -2000 && rpt->altitude_raw <= 2001) {
          DictSafeSetItem(rpt_dict, "altitude", rpt->altitude);
        } else {
          DictSafeSetItem(rpt_dict, "altitude", Py_None);
        }

        DictSafeSetItem(rpt_dict, "owner", rpt->owner);
        DictSafeSetItem(rpt_dict, "timeout", rpt->timeout);
        DictSafeSetItem(rpt_dict, "spare2", rpt->spare2);
      }
      break;
    case AIS8_367_33_SENSOR_STATION:
      {
        Ais8_367_33_Station *rpt =
            dynamic_cast<Ais8_367_33_Station *>(msg.reports[rpt_num].get());
        assert(rpt != nullptr);
        ais8_367_33_append_pydict_sensor_hdr(rpt_dict, rpt);

        DictSafeSetItem(rpt_dict, "name", rpt->name);
        DictSafeSetItem(rpt_dict, "spare2", rpt->spare2);
      }
      break;
    case AIS8_367_33_SENSOR_WIND:
      {
        Ais8_367_33_Wind *rpt =
            dynamic_cast<Ais8_367_33_Wind *>(msg.reports[rpt_num].get());
        assert(rpt != nullptr);
        ais8_367_33_append_pydict_sensor_hdr(rpt_dict, rpt);

        if (rpt->wind_speed >= 0 && rpt->wind_speed <= 121) {
          DictSafeSetItem(rpt_dict, "wind_speed", rpt->wind_speed);
        } else {
          DictSafeSetItem(rpt_dict, "wind_speed", Py_None);
        }

        if (rpt->wind_gust >= 0 && rpt->wind_gust <= 121) {
          DictSafeSetItem(rpt_dict, "wind_gust", rpt->wind_gust);
        } else {
          DictSafeSetItem(rpt_dict, "wind_gust", Py_None);
        }

        if (rpt->wind_dir >= 0 && rpt->wind_dir <= 359) {
          DictSafeSetItem(rpt_dict, "wind_dir", rpt->wind_dir);
        } else {
          DictSafeSetItem(rpt_dict, "wind_dir", Py_None);
        }

        if (rpt->wind_gust_dir >= 0 && rpt->wind_gust_dir <= 359) {
          DictSafeSetItem(rpt_dict, "wind_gust_dir", rpt->wind_gust_dir);
        } else {
          DictSafeSetItem(rpt_dict, "wind_gust_dir", Py_None);
        }

        DictSafeSetItem(rpt_dict, "sensor_type", rpt->sensor_type);

        if (rpt->wind_forecast >= 0 && rpt->wind_forecast <= 121) {
          DictSafeSetItem(rpt_dict, "wind_forecast", rpt->wind_forecast);
        } else {
          DictSafeSetItem(rpt_dict, "wind_forecast", Py_None);
        }

        if (rpt->wind_gust_forecast >= 0 && rpt->wind_gust_forecast <= 121) {
          DictSafeSetItem(rpt_dict, "wind_gust_forecast", rpt->wind_gust_forecast);
        } else {
          DictSafeSetItem(rpt_dict, "wind_gust_forecast", Py_None);
        }

        if (rpt->wind_dir_forecast >= 0 && rpt->wind_dir_forecast <= 359) {
          DictSafeSetItem(rpt_dict, "wind_dir_forecast", rpt->wind_dir_forecast);
        } else {
          DictSafeSetItem(rpt_dict, "wind_dir_forecast", Py_None);
        }

        if (rpt->utc_day_forecast != 0) {
          DictSafeSetItem(rpt_dict, "utc_day_forecast", rpt->utc_day_forecast);
        } else {
          DictSafeSetItem(rpt_dict, "utc_day_forecast", Py_None);
        }

        if (rpt->utc_hour_forecast >= 0 && rpt->utc_hour_forecast <= 23) {
          DictSafeSetItem(rpt_dict, "utc_hour_forecast", rpt->utc_hour_forecast);
        } else {
          DictSafeSetItem(rpt_dict, "utc_hour_forecast", Py_None);
        }

        if (rpt->utc_min_forecast >= 0 && rpt->utc_min_forecast <= 59) {
          DictSafeSetItem(rpt_dict, "utc_min_forecast", rpt->utc_min_forecast);
        } else {
          DictSafeSetItem(rpt_dict, "utc_min_forecast", Py_None);
        }

        DictSafeSetItem(rpt_dict, "duration", rpt->duration);
        DictSafeSetItem(rpt_dict, "spare2", rpt->spare2);
      }
      break;
    case AIS8_367_33_SENSOR_WATER_LEVEL:
      {
        Ais8_367_33_WaterLevel *rpt =
            dynamic_cast<Ais8_367_33_WaterLevel *>(msg.reports[rpt_num].get());
        assert(rpt != nullptr);
        ais8_367_33_append_pydict_sensor_hdr(rpt_dict, rpt);
        DictSafeSetItem(rpt_dict, "type", rpt->type);

        if (rpt->level != 32768) {
          DictSafeSetItem(rpt_dict, "level", rpt->level);
        } else {
          DictSafeSetItem(rpt_dict, "level", Py_None);
        }

        DictSafeSetItem(rpt_dict, "trend", rpt->trend);
        DictSafeSetItem(rpt_dict, "vdatum", rpt->vdatum);
        DictSafeSetItem(rpt_dict, "sensor_type", rpt->sensor_type);
        DictSafeSetItem(rpt_dict, "forecast_type", rpt->forecast_type);

        if (rpt->level_forecast != 32768) {
          DictSafeSetItem(rpt_dict, "level_forecast", rpt->level_forecast);
        } else {
          DictSafeSetItem(rpt_dict, "level_forecast", Py_None);
        }

        DictSafeSetItem(rpt_dict, "utc_day_forecast", rpt->utc_day_forecast);
        DictSafeSetItem(rpt_dict, "utc_hour_forecast", rpt->utc_hour_forecast);
        DictSafeSetItem(rpt_dict, "utc_min_forecast", rpt->utc_min_forecast);
        DictSafeSetItem(rpt_dict, "duration", rpt->duration);
        DictSafeSetItem(rpt_dict, "spare2", rpt->spare2);
      }
      break;
    case AIS8_367_33_SENSOR_CURR_2D:
      {
        Ais8_367_33_Curr2D *rpt =
            dynamic_cast<Ais8_367_33_Curr2D *>(msg.reports[rpt_num].get());
        assert(rpt != nullptr);
        ais8_367_33_append_pydict_sensor_hdr(rpt_dict, rpt);
        DictSafeSetItem(rpt_dict, "type", rpt->type);
        DictSafeSetItem(rpt_dict, "spare2", rpt->spare2);

        PyObject *curr_list = PyList_New(3);
        DictSafeSetItem(rpt_dict, "currents", curr_list);
        for (size_t idx = 0; idx < 3; idx++) {
          PyObject *curr_dict = PyDict_New();

          if (rpt->currents[idx].speed_raw <= 246) {
            DictSafeSetItem(curr_dict, "speed", rpt->currents[idx].speed);
          } else {
            DictSafeSetItem(curr_dict, "speed", Py_None);
          }

          if (rpt->currents[idx].dir <= 359) {
            DictSafeSetItem(curr_dict, "dir", rpt->currents[idx].dir);
          } else {
            DictSafeSetItem(curr_dict, "dir", Py_None);
          }

          if (rpt->currents[idx].depth <= 361) {
            DictSafeSetItem(curr_dict, "depth", rpt->currents[idx].depth);
          } else {
            DictSafeSetItem(curr_dict, "depth", Py_None);
          }

          PyList_SetItem(curr_list, idx, curr_dict);
        }
      }
      break;
    case AIS8_367_33_SENSOR_CURR_3D:
      {
        Ais8_367_33_Curr3D *rpt =
            dynamic_cast<Ais8_367_33_Curr3D *>(msg.reports[rpt_num].get());
        assert(rpt != nullptr);
        ais8_367_33_append_pydict_sensor_hdr(rpt_dict, rpt);
        DictSafeSetItem(rpt_dict, "type", rpt->type);
        DictSafeSetItem(rpt_dict, "spare2", rpt->spare2);

        PyObject *curr_list = PyList_New(2);
        DictSafeSetItem(rpt_dict, "currents", curr_list);
        for (size_t idx = 0; idx < 2; idx++) {
          PyObject *curr_dict = PyDict_New();
          PyList_SetItem(curr_list, idx, curr_dict);

          if (rpt->currents[idx].north_raw >= -251 && rpt->currents[idx].north_raw <= 251) {
            DictSafeSetItem(curr_dict, "north", rpt->currents[idx].north);
          } else {
            DictSafeSetItem(curr_dict, "north", Py_None);
          }

          if (rpt->currents[idx].east_raw >= -251 && rpt->currents[idx].east_raw <= 251) {
            DictSafeSetItem(curr_dict, "east", rpt->currents[idx].east);
          } else {
            DictSafeSetItem(curr_dict, "east", Py_None);
          }

          if (rpt->currents[idx].up_raw >= -251 && rpt->currents[idx].up_raw <= 251) {
            DictSafeSetItem(curr_dict, "up", rpt->currents[idx].up);
          } else {
            DictSafeSetItem(curr_dict, "up", Py_None);
          }

          if (rpt->currents[idx].depth >= 0 && rpt->currents[idx].depth <= 361) {
            DictSafeSetItem(curr_dict, "depth", rpt->currents[idx].depth);
          } else {
            DictSafeSetItem(curr_dict, "depth", Py_None);
          }
        }
      }
      break;
    case AIS8_367_33_SENSOR_HORZ_FLOW:
      {
        Ais8_367_33_HorzFlow *rpt =
            dynamic_cast<Ais8_367_33_HorzFlow *>(msg.reports[rpt_num].get());
        assert(rpt != nullptr);
        ais8_367_33_append_pydict_sensor_hdr(rpt_dict, rpt);
        DictSafeSetItem(rpt_dict, "spare2", rpt->spare2);

        if (rpt->bearing <= 359) {
          DictSafeSetItem(rpt_dict, "bearing", rpt->bearing);
        } else {
            DictSafeSetItem(rpt_dict, "bearing", Py_None);
        }

        DictSafeSetItem(rpt_dict, "type", rpt->type);

        PyObject *curr_list = PyList_New(2);
        DictSafeSetItem(rpt_dict, "currents", curr_list);
        for (size_t idx = 0; idx < 2; idx++) {
          PyObject *curr_dict = PyDict_New();

          if (rpt->currents[idx].dist <= 361) {
            DictSafeSetItem(curr_dict, "dist", rpt->currents[idx].dist);
           } else {
            DictSafeSetItem(curr_dict, "dist", Py_None);
          }

          if (rpt->currents[idx].speed_raw <= 246) {
            DictSafeSetItem(curr_dict, "speed", rpt->currents[idx].speed);
           } else {
            DictSafeSetItem(curr_dict, "speed", Py_None);
          }

          if (rpt->currents[idx].dir <= 359) {
            DictSafeSetItem(curr_dict, "dir", rpt->currents[idx].dir);
           } else {
            DictSafeSetItem(curr_dict, "dir", Py_None);
          }

          if (rpt->currents[idx].level <= 361) {
            DictSafeSetItem(curr_dict, "level", rpt->currents[idx].level);
           } else {
            DictSafeSetItem(curr_dict, "level", Py_None);
          }

          PyList_SetItem(curr_list, idx, curr_dict);
        }
      }
      break;
    case AIS8_367_33_SENSOR_SEA_STATE:
      {
        Ais8_367_33_SeaState *rpt =
            dynamic_cast<Ais8_367_33_SeaState *>(msg.reports[rpt_num].get());
        assert(rpt != nullptr);
        ais8_367_33_append_pydict_sensor_hdr(rpt_dict, rpt);

        if (rpt->swell_height_raw <= 246) {
          DictSafeSetItem(rpt_dict, "swell_height", rpt->swell_height);
        } else {
          DictSafeSetItem(rpt_dict, "swell_height", Py_None);
        }

        if (rpt->swell_period <= 60) {
          DictSafeSetItem(rpt_dict, "swell_period", rpt->swell_period);
        } else {
          DictSafeSetItem(rpt_dict, "swell_period", Py_None);
        }

        if (rpt->swell_dir <= 359) {
          DictSafeSetItem(rpt_dict, "swell_dir", rpt->swell_dir);
        } else {
          DictSafeSetItem(rpt_dict, "swell_dir", Py_None);
        }

        DictSafeSetItem(rpt_dict, "sea_state", rpt->sea_state);
        DictSafeSetItem(rpt_dict, "swell_sensor_type", rpt->swell_sensor_type);

        if (rpt->water_temp_raw <= 600) {
          DictSafeSetItem(rpt_dict, "water_temp", rpt->water_temp);
        } else {
          DictSafeSetItem(rpt_dict, "water_temp", Py_None);
        }

        if (rpt->water_temp_depth_raw <= 121) {
          DictSafeSetItem(rpt_dict, "water_temp_depth", rpt->water_temp_depth);
        } else {
          DictSafeSetItem(rpt_dict, "water_temp_depth", Py_None);
        }

        DictSafeSetItem(rpt_dict, "water_sensor_type", rpt->water_sensor_type);

        if (rpt->wave_height_raw <= 246) {
          DictSafeSetItem(rpt_dict, "wave_height", rpt->wave_height);
        } else {
          DictSafeSetItem(rpt_dict, "wave_height", Py_None);
        }

        if (rpt->wave_period <= 60) {
          DictSafeSetItem(rpt_dict, "wave_period", rpt->wave_period);
        } else {
          DictSafeSetItem(rpt_dict, "wave_period", Py_None);
        }

        if (rpt->wave_dir <= 359) {
          DictSafeSetItem(rpt_dict, "wave_dir", rpt->wave_dir);
        } else {
          DictSafeSetItem(rpt_dict, "wave_dir", Py_None);
        }

        DictSafeSetItem(rpt_dict, "wave_sensor_type", rpt->wave_sensor_type);

        if (rpt->salinity_raw <= 501) {
          DictSafeSetItem(rpt_dict, "salinity", rpt->salinity);
        } else {
          DictSafeSetItem(rpt_dict, "salinity", Py_None);
        }

      }
      break;
    case AIS8_367_33_SENSOR_SALINITY:
      {
        Ais8_367_33_Salinity *rpt =
            dynamic_cast<Ais8_367_33_Salinity *>(msg.reports[rpt_num].get());
        assert(rpt != nullptr);
        ais8_367_33_append_pydict_sensor_hdr(rpt_dict, rpt);

        if (rpt->water_temp_raw <= 600) {
          DictSafeSetItem(rpt_dict, "water_temp", rpt->water_temp);
        } else {
          DictSafeSetItem(rpt_dict, "water_temp", Py_None);
        }

        if (rpt->conductivity_raw <= 701) {
          DictSafeSetItem(rpt_dict, "conductivity", rpt->conductivity);
        } else {
          DictSafeSetItem(rpt_dict, "conductivity", Py_None);
        }

        if (rpt->pressure_raw <= 60001) {
          DictSafeSetItem(rpt_dict, "pressure", rpt->pressure);
        } else {
          DictSafeSetItem(rpt_dict, "pressure", Py_None);
        }

        if (rpt->salinity_raw <= 501) {
          DictSafeSetItem(rpt_dict, "salinity", rpt->salinity);
        } else {
          DictSafeSetItem(rpt_dict, "salinity", Py_None);
        }

        DictSafeSetItem(rpt_dict, "salinity_type", rpt->salinity_type);
        DictSafeSetItem(rpt_dict, "sensor_type", rpt->sensor_type);
        DictSafeSetItem(rpt_dict, "spare0", rpt->spare2[0]);
        DictSafeSetItem(rpt_dict, "spare1", rpt->spare2[1]);
      }
      break;
    case AIS8_367_33_SENSOR_WX:
      {
        Ais8_367_33_Wx *rpt =
            dynamic_cast<Ais8_367_33_Wx *>(msg.reports[rpt_num].get());
        assert(rpt != nullptr);
        ais8_367_33_append_pydict_sensor_hdr(rpt_dict, rpt);

        if (rpt->air_temp_raw >= -600 && rpt->air_temp_raw <= 600) {
          DictSafeSetItem(rpt_dict, "air_temp", rpt->air_temp);
        } else {
          DictSafeSetItem(rpt_dict, "air_temp", Py_None);
        }

        DictSafeSetItem(rpt_dict, "air_temp_sensor_type",
                        rpt->air_temp_sensor_type);
        DictSafeSetItem(rpt_dict, "precip", rpt->precip);

        if (rpt->horz_vis_raw <= 241) {
          DictSafeSetItem(rpt_dict, "horz_vis", rpt->horz_vis);
        } else {
          DictSafeSetItem(rpt_dict, "horz_vis", Py_None);
        }

        // Not sure how to map 702 and 703 to Python.
        if (rpt->dew_point_raw <= 700) {
          DictSafeSetItem(rpt_dict, "dew_point", rpt->dew_point);
        } else {
          DictSafeSetItem(rpt_dict, "dew_point", Py_None);
        }

        DictSafeSetItem(rpt_dict, "dew_point_type", rpt->dew_point_type);

        if (rpt->air_pressure_raw <= 402) {
          DictSafeSetItem(rpt_dict, "air_pressure", rpt->air_pressure);
        } else {
          DictSafeSetItem(rpt_dict, "air_pressure", Py_None);
        }

        DictSafeSetItem(rpt_dict, "air_pressure_trend",
                        rpt->air_pressure_trend);
        DictSafeSetItem(rpt_dict, "air_pressure_sensor_type", rpt->air_pressure_sensor_type);

        if (rpt->salinity_raw <= 501) {
          DictSafeSetItem(rpt_dict, "salinity", rpt->salinity);
        } else {
          DictSafeSetItem(rpt_dict, "salinity", Py_None);
        }

        DictSafeSetItem(rpt_dict, "spare2", rpt->spare2);
      }
      break;
    case AIS8_367_33_SENSOR_AIR_GAP:
      {
        Ais8_367_33_AirGap *rpt =
            dynamic_cast<Ais8_367_33_AirGap *>(msg.reports[rpt_num].get());
        assert(rpt != nullptr);
        ais8_367_33_append_pydict_sensor_hdr(rpt_dict, rpt);

        if (rpt->air_draught != 0) {
          DictSafeSetItem(rpt_dict, "air_draught", rpt->air_draught);
        } else {
          DictSafeSetItem(rpt_dict, "air_draught", Py_None);
        }

        if (rpt->air_gap != 0) {
          DictSafeSetItem(rpt_dict, "air_gap", rpt->air_gap);
        } else {
          DictSafeSetItem(rpt_dict, "air_gap", Py_None);
        }

        DictSafeSetItem(rpt_dict, "air_gap_trend", rpt->air_gap_trend);

        if (rpt->predicted_air_gap != 0) {
          DictSafeSetItem(rpt_dict, "predicted_air_gap", rpt->predicted_air_gap);
        } else {
          DictSafeSetItem(rpt_dict, "predicted_air_gap", Py_None);
        }

        if (rpt->utc_day_forecast != 0) {
          DictSafeSetItem(rpt_dict, "utc_day_forecast", rpt->utc_day_forecast);
        } else {
          DictSafeSetItem(rpt_dict, "utc_day_forecast", Py_None);
        }

        if (rpt->utc_hour_forecast <= 23) {
          DictSafeSetItem(rpt_dict, "utc_hour_forecast", rpt->utc_hour_forecast);
        } else {
          DictSafeSetItem(rpt_dict, "utc_hour_forecast", Py_None);
        }

        if (rpt->utc_min_forecast <= 59) {
          DictSafeSetItem(rpt_dict, "utc_min_forecast", rpt->utc_min_forecast);
        } else {
          DictSafeSetItem(rpt_dict, "utc_min_forecast", Py_None);
        }

        DictSafeSetItem(rpt_dict, "spare2", rpt->spare2);
      }
      break;
    case AIS8_367_33_SENSOR_WIND_REPORT_2:
      {
        Ais8_367_33_Wind_V2 *rpt =
            dynamic_cast<Ais8_367_33_Wind_V2 *>(msg.reports[rpt_num].get());
        assert(rpt != nullptr);
        ais8_367_33_append_pydict_sensor_hdr(rpt_dict, rpt);

        if (rpt->wind_speed <= 121) {
          DictSafeSetItem(rpt_dict, "wind_speed", rpt->wind_speed);
        } else {
          DictSafeSetItem(rpt_dict, "wind_speed", Py_None);
        }

        if (rpt->wind_gust <= 121) {
          DictSafeSetItem(rpt_dict, "wind_gust", rpt->wind_gust);
        } else {
          DictSafeSetItem(rpt_dict, "wind_gust", Py_None);
        }

        if (rpt->wind_dir <= 359) {
          DictSafeSetItem(rpt_dict, "wind_dir", rpt->wind_dir);
        } else {
          DictSafeSetItem(rpt_dict, "wind_dir", Py_None);
        }

        if (rpt->averaging_time >= 1 && rpt->averaging_time <= 61) {
          DictSafeSetItem(rpt_dict, "averaging_time", rpt->averaging_time);
        } else {
          DictSafeSetItem(rpt_dict, "averaging_time", Py_None);
        }

        DictSafeSetItem(rpt_dict, "sensor_type", rpt->sensor_type);

        if (rpt->wind_speed_forecast <= 121) {
          DictSafeSetItem(rpt_dict, "wind_speed_forecast", rpt->wind_speed_forecast);
        } else {
          DictSafeSetItem(rpt_dict, "wind_speed_forecast", Py_None);
        }

        if (rpt->wind_gust_forecast <= 121) {
          DictSafeSetItem(rpt_dict, "wind_gust_forecast", rpt->wind_gust_forecast);
        } else {
          DictSafeSetItem(rpt_dict, "wind_gust_forecast", Py_None);
        }

        if (rpt->wind_dir_forecast <= 359) {
          DictSafeSetItem(rpt_dict, "wind_dir_forecast", rpt->wind_dir_forecast);
        } else {
          DictSafeSetItem(rpt_dict, "wind_dir_forecast", Py_None);
        }

        if (rpt->utc_hour_forecast <= 23) {
          DictSafeSetItem(rpt_dict, "utc_hour_forecast", rpt->utc_hour_forecast);
        } else {
          DictSafeSetItem(rpt_dict, "utc_hour_forecast", Py_None);
        }

        if (rpt->utc_min_forecast <= 59) {
          DictSafeSetItem(rpt_dict, "utc_min_forecast", rpt->utc_min_forecast);
        } else {
          DictSafeSetItem(rpt_dict, "utc_min_forecast", Py_None);
        }

        DictSafeSetItem(rpt_dict, "duration", rpt->duration);
        DictSafeSetItem(rpt_dict, "spare2", rpt->spare2);
      }
      break;
    case AIS8_367_33_SENSOR_RESERVED_12:
    case AIS8_367_33_SENSOR_RESERVED_13:
    case AIS8_367_33_SENSOR_RESERVED_14:
    case AIS8_367_33_SENSOR_RESERVED_15:
    default:
      {}  // TODO(schwehr): mark a bad sensor type or raise exception
    }
  }
  return AIS_OK;
}

// AIS Binary broadcast messages.  There will be a huge number of subtypes
// If we don't know how to decode it, just return the dac, fi
PyObject*
ais8_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  assert(pad < 6);

  Ais8 msg(nmea_payload, pad);
  if (msg.had_error() && msg.get_error() != AIS_UNINITIALIZED) {
    PyErr_Format(ais_py_exception, "Ais8: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);
  DictSafeSetItem(dict, "spare", msg.spare);
  DictSafeSetItem(dict, "dac", msg.dac);
  DictSafeSetItem(dict, "fi", msg.fi);

  AIS_STATUS status = AIS_UNINITIALIZED;

  switch (msg.dac) {
  case AIS_DAC_1_INTERNATIONAL:  // IMO.
    switch (msg.fi) {
      // See: ITU-R.M.1371-3 IFM messages Annex 5, Section 5 and IMO Circ 289
    case AIS_FI_8_1_0_TEXT:
      status = ais8_1_0_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_1_11_MET_HYDRO:  // Not to be used after 1 Jan 2013.
      status = ais8_1_11_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_1_13_FAIRWAY_CLOSED:  // Not to be used after 1 Jan 2013.
      // TODO(schwehr): untested - no messages found
      status = ais8_1_13_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_1_15_SHIP_AND_VOYAGE:  // Not after 1 Jan 2013.
      // TODO(schwehr): untested - no messages found
      status = ais8_1_15_append_pydict(nmea_payload, dict, pad);
      break;
      // 16 has conflicting definition in the old 1371-1: VTS targets.
    case AIS_FI_8_1_16_PERSONS_ON_BOARD:  // Not to be used after 1 Jan 2013.
      status = ais8_1_16_append_pydict(nmea_payload, dict, pad);
      break;
      // 17 has conflicting definitions in 1371-1: IFM 17: Ship waypoints
      // and/or route plan report.
    case AIS_FI_8_1_17_VTS_TARGET:  // Not to be used after Jan 2013.
      // TODO(schwehr): Untested. no messages found.
      status = ais8_1_17_append_pydict(nmea_payload, dict, pad);
      break;
      // ITU 1371-1 conflict: IFM 19: Extended ship static and voyage.
    case AIS_FI_8_1_19_TRAFFIC_SIGNAL:
      // TODO(schwehr): Untested. No messages found.
      status = ais8_1_19_append_pydict(nmea_payload, dict, pad);
      DictSafeSetItem(dict, "parsed", true);
      break;
      // 20:  Berthing data.
    case AIS_FI_8_1_21_WEATHER_OBS:
      // TODO(schwehr): untested - no messages found
      status = ais8_1_21_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_1_22_AREA_NOTICE:
      status = ais8_1_22_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_1_24_SHIP_AND_VOYAGE:
      status = ais8_1_24_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_1_26_SENSOR:
      status = ais8_1_26_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_1_27_ROUTE:
      status = ais8_1_27_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_1_29_TEXT:
      status = ais8_1_29_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_1_31_MET_HYDRO:
      status = ais8_1_31_append_pydict(nmea_payload, dict, pad);
      break;
      // ITU 1371-1 only: 3.10 - IFM 40: Number of persons on board.
    default:
      DictSafeSetItem(dict, "parsed", false);
      break;
    }
    break;
  case AIS_DAC_200_RIS:
    switch (msg.fi) {
    case AIS_FI_8_200_10_RIS_SHIP_AND_VOYAGE:
      status = ais8_200_10_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_200_21_RIS_ETA_AT_LOCK_BRIDGE_TERMINAL:
      status = ais8_200_21_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_200_22_RIS_RTA_AT_LOCK_BRIDGE_TERMINAL:
      status = ais8_200_22_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_200_23_RIS_EMMA_WARNING:
      status = ais8_200_23_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_1_24_SHIP_AND_VOYAGE:
      status = ais8_200_24_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_200_40_RIS_ATON_SIGNAL_STATUS:
      status = ais8_200_40_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_200_55_RIS_PERSONS_ON_BOARD:
      status = ais8_200_55_append_pydict(nmea_payload, dict, pad);
      break;
    default:
      DictSafeSetItem(dict, "parsed", false);
      break;
    }
    break;
    // TODO(schwehr): AIS_FI_8_366_22_AREA_NOTICE.
  case 367:  // United states.
    switch (msg.fi) {
    case 22:  // USCG Area Notice 2012 (v5?).
      ais8_367_22_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_367_23_SSW:
      status = ais8_367_23_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_367_24_SSW_SMALL:
      status = ais8_367_24_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_367_25_SSW_TINY:
      status = ais8_367_25_append_pydict(nmea_payload, dict, pad);
      break;
    case AIS_FI_8_367_33_ENVIRONMENTAL:
      status = ais8_367_33_append_pydict(nmea_payload, dict, pad);
      break;
    default:
      DictSafeSetItem(dict, "parsed", false);
      break;
    }
    break;
  default:
    DictSafeSetItem(dict, "parsed", false);
    // TODO(schwehr): raise exception or return standin?
  }

  if (status != AIS_OK) {
    Py_DECREF(dict);
    PyErr_Format(ais_py_exception, "Ais8: %s",
                 AIS_STATUS_STRINGS[status]);
    return nullptr;
  }

  return dict;
}

// Aircraft position report
PyObject*
ais9_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  Ais9 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais9: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  DictSafeSetItem(dict, "alt", msg.alt);
  DictSafeSetItem(dict, "sog", msg.sog);

  DictSafeSetItem(dict, "position_accuracy", msg.position_accuracy);
  DictSafeSetItem(dict, "x", "y", msg.position);

  DictSafeSetItem(dict, "cog", msg.cog);
  DictSafeSetItem(dict, "timestamp", msg.timestamp);
  DictSafeSetItem(dict, "alt_sensor", msg.alt_sensor);
  DictSafeSetItem(dict, "spare", msg.spare);
  DictSafeSetItem(dict, "dte", msg.dte);
  DictSafeSetItem(dict, "spare2", msg.spare2);
  DictSafeSetItem(dict, "assigned_mode", msg.assigned_mode);
  DictSafeSetItem(dict, "raim", msg.raim);

  DictSafeSetItem(dict, "sync_state", msg.sync_state);

  if (0 == msg.commstate_flag) {
    // SOTMDA
    DictSafeSetItem(dict, "slot_timeout", msg.slot_timeout);

    if (msg.received_stations_valid)
      DictSafeSetItem(dict, "received_stations", msg.received_stations);
    if (msg.slot_number_valid)
      DictSafeSetItem(dict, "slot_number", msg.slot_number);
    if (msg.utc_valid) {
      DictSafeSetItem(dict, "utc_hour", msg.utc_hour);
      DictSafeSetItem(dict, "utc_min", msg.utc_min);
      DictSafeSetItem(dict, "utc_spare", msg.utc_spare);
    }
    if (msg.slot_offset_valid)
      DictSafeSetItem(dict, "slot_offset", msg.slot_offset);
  } else {
    // ITDMA
    DictSafeSetItem(dict, "slot_increment", msg.slot_increment);
    DictSafeSetItem(dict, "slots_to_allocate", msg.slots_to_allocate);
    DictSafeSetItem(dict, "keep_flag", msg.keep_flag);
  }

  return dict;
}

// 10 - ':' - UTC and date inquiry
PyObject*
ais10_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  assert(pad < 6);

  Ais10 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais10: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  DictSafeSetItem(dict, "spare", msg.spare);
  DictSafeSetItem(dict, "dest_mmsi", msg.dest_mmsi);
  DictSafeSetItem(dict, "spare2", msg.spare2);

  return dict;
}

// msg 11 ';' - See msg 4_11

// 12 - '<' - Addressed safety related text
PyObject*
ais12_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  Ais12 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais12: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  DictSafeSetItem(dict, "seq_num", msg.seq_num);
  DictSafeSetItem(dict, "dest_mmsi", msg.dest_mmsi);
  DictSafeSetItem(dict, "retransmitted", msg.retransmitted);

  DictSafeSetItem(dict, "spare", msg.spare);
  DictSafeSetItem(dict, "text", msg.text);

  return dict;
}

// msg 13 - See msg 7

// 14 - '>' - Safety broadcast text
PyObject*
ais14_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  Ais14 msg(nmea_payload, pad);

  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais14: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);
  DictSafeSetItem(dict, "spare", msg.spare);
  DictSafeSetItem(dict, "text", msg.text);
  DictSafeSetItem(dict, "spare2", msg.spare2);

  return dict;
}

// 15 - '?' - Interrogation
PyObject*
ais15_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  Ais15 msg(nmea_payload, pad);

  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais15: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  DictSafeSetItem(dict, "spare", msg.spare);
  DictSafeSetItem(dict, "mmsi_1", msg.mmsi_1);
  DictSafeSetItem(dict, "msg_1_1", msg.msg_1_1);
  DictSafeSetItem(dict, "slot_offset_1_1", msg.slot_offset_1_1);

  DictSafeSetItem(dict, "spare2", msg.spare2);
  DictSafeSetItem(dict, "dest_msg_1_2", msg.dest_msg_1_2);
  DictSafeSetItem(dict, "slot_offset_1_2", msg.slot_offset_1_2);

  DictSafeSetItem(dict, "spare3", msg.spare3);
  DictSafeSetItem(dict, "mmsi_2", msg.mmsi_2);
  DictSafeSetItem(dict, "msg_2", msg.msg_2);
  DictSafeSetItem(dict, "slot_offset_2", msg.slot_offset_2);
  DictSafeSetItem(dict, "spare4", msg.spare4);

  return dict;
}

// 16 - '@' - Assigned mode command
PyObject*
ais16_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  Ais16 msg(nmea_payload, pad);

  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais16: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  DictSafeSetItem(dict, "spare", msg.spare);
  DictSafeSetItem(dict, "dest_mmsi_a", msg.dest_mmsi_a);
  DictSafeSetItem(dict, "offset_a", msg.offset_a);
  DictSafeSetItem(dict, "inc_a", msg.inc_a);

  if (-1 != msg.spare2) DictSafeSetItem(dict, "spare2", msg.spare2);
  if (-1 != msg.dest_mmsi_b) {
    DictSafeSetItem(dict, "dest_mmsi_b", msg.dest_mmsi_b);
    DictSafeSetItem(dict, "offset_b", msg.offset_b);
    DictSafeSetItem(dict, "inc_b", msg.inc_b);
  }

  return dict;
}

// 17 - 'A' - GNSS differential - TODO(schwehr): incomplete
PyObject*
ais17_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  Ais17 msg(nmea_payload, pad);

  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais17: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  DictSafeSetItem(dict, "spare", msg.spare);
  DictSafeSetItem(dict, "x", "y", msg.position);
  DictSafeSetItem(dict, "spare2", msg.spare2);

  return dict;
}

// 18 - 'B' - Class B position report.
PyObject*
ais18_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  Ais18 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais18: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  DictSafeSetItem(dict, "spare", msg.spare);
  DictSafeSetItem(dict, "sog", msg.sog);
  DictSafeSetItem(dict, "position_accuracy", msg.position_accuracy);
  DictSafeSetItem(dict, "x", "y", msg.position);
  DictSafeSetItem(dict, "cog", msg.cog);
  DictSafeSetItem(dict, "true_heading", msg.true_heading);
  DictSafeSetItem(dict, "timestamp", msg.timestamp);

  DictSafeSetItem(dict, "spare2", msg.spare2);

  DictSafeSetItem(dict, "unit_flag", msg.unit_flag);
  DictSafeSetItem(dict, "display_flag", msg.display_flag);
  DictSafeSetItem(dict, "dsc_flag", msg.dsc_flag);
  DictSafeSetItem(dict, "band_flag", msg.band_flag);
  DictSafeSetItem(dict, "m22_flag", msg.m22_flag);
  DictSafeSetItem(dict, "mode_flag", msg.mode_flag);

  DictSafeSetItem(dict, "raim", msg.raim);

  DictSafeSetItem(dict, "commstate_flag", msg.commstate_flag);
  if (msg.slot_timeout_valid) {
    DictSafeSetItem(dict, "slot_timeout", msg.slot_timeout);
  }
  if (msg.slot_offset_valid) {
        DictSafeSetItem(dict, "slot_offset", msg.slot_offset);
  }
  if (msg.utc_valid) {
    DictSafeSetItem(dict, "utc_hour", msg.utc_hour);
    DictSafeSetItem(dict, "utc_min", msg.utc_min);
    DictSafeSetItem(dict, "utc_spare", msg.utc_spare);
  }
  if (msg.slot_number_valid) {
    DictSafeSetItem(dict, "slot_number", msg.slot_number);
  }
  if (msg.received_stations_valid) {
    DictSafeSetItem(dict, "received_stations", msg.received_stations);
  }

  // ITDMA
  if (msg.slot_increment_valid) {
    DictSafeSetItem(dict, "slot_increment", msg.slot_increment);
    DictSafeSetItem(dict, "slots_to_allocate", msg.slots_to_allocate);
    DictSafeSetItem(dict, "keep_flag", msg.keep_flag);
  }

  if (msg.commstate_cs_fill_valid) {
    DictSafeSetItem(dict, "commstate_cs_fill", msg.commstate_cs_fill);
  }

  return dict;
}

// 19 - 'C' - Class B combined position report and ship data
PyObject*
ais19_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  Ais19 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais19: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  DictSafeSetItem(dict, "spare", msg.spare);
  DictSafeSetItem(dict, "sog", msg.sog);
  DictSafeSetItem(dict, "position_accuracy", msg.position_accuracy);
  DictSafeSetItem(dict, "x", "y", msg.position);
  DictSafeSetItem(dict, "cog", msg.cog);
  DictSafeSetItem(dict, "true_heading", msg.true_heading);
  DictSafeSetItem(dict, "timestamp", msg.timestamp);

  DictSafeSetItem(dict, "spare2", msg.spare2);

  DictSafeSetItem(dict, "name", msg.name);
  DictSafeSetItem(dict, "type_and_cargo", msg.type_and_cargo);
  DictSafeSetItem(dict, "dim_a", msg.dim_a);
  DictSafeSetItem(dict, "dim_b", msg.dim_b);
  DictSafeSetItem(dict, "dim_c", msg.dim_c);
  DictSafeSetItem(dict, "dim_d", msg.dim_d);
  DictSafeSetItem(dict, "fix_type", msg.fix_type);

  DictSafeSetItem(dict, "raim", msg.raim);

  DictSafeSetItem(dict, "dte", msg.dte);
  DictSafeSetItem(dict, "assigned_mode", msg.assigned_mode);
  DictSafeSetItem(dict, "spare3", msg.spare3);

  return dict;
}

// 20 - 'D' - data link management
PyObject*
ais20_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  Ais20 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais20: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);
  DictSafeSetItem(dict, "spare", msg.spare);

  int list_size = 1;
  if (msg.group_valid_4) list_size = 4;
  else if (msg.group_valid_3) list_size = 3;
  else if (msg.group_valid_2) list_size = 2;

  PyObject *list = PyList_New(list_size);

  {
    PyObject *reservation = PyDict_New();
    DictSafeSetItem(reservation, "offset", msg.offset_1);
    DictSafeSetItem(reservation, "num_slots", msg.num_slots_1);
    DictSafeSetItem(reservation, "timeout", msg.timeout_1);
    DictSafeSetItem(reservation, "incr", msg.incr_1);
    PyList_SetItem(list, 0, reservation);
  }

  if (msg.group_valid_2) {
    PyObject *reservation = PyDict_New();
    DictSafeSetItem(reservation, "offset", msg.offset_2);
    DictSafeSetItem(reservation, "num_slots", msg.num_slots_2);
    DictSafeSetItem(reservation, "timeout", msg.timeout_2);
    DictSafeSetItem(reservation, "incr", msg.incr_2);
    PyList_SetItem(list, 1, reservation);
  }

  if (msg.group_valid_3) {
    PyObject *reservation = PyDict_New();
    DictSafeSetItem(reservation, "offset", msg.offset_3);
    DictSafeSetItem(reservation, "num_slots", msg.num_slots_3);
    DictSafeSetItem(reservation, "timeout", msg.timeout_3);
    DictSafeSetItem(reservation, "incr", msg.incr_3);
    PyList_SetItem(list, 2, reservation);
  }

  if (msg.group_valid_4) {
    PyObject *reservation = PyDict_New();
    DictSafeSetItem(reservation, "offset", msg.offset_4);
    DictSafeSetItem(reservation, "num_slots", msg.num_slots_4);
    DictSafeSetItem(reservation, "timeout", msg.timeout_4);
    DictSafeSetItem(reservation, "incr", msg.incr_4);
    PyList_SetItem(list, 3, reservation);
  }

  PyDict_SetItemString(dict, "reservations", list);
  Py_DECREF(list);

  return dict;
}


// 21 - 'E' - ATON Aid to Navigation
PyObject*
ais21_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  Ais21 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais21: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);
  DictSafeSetItem(dict, "spare", msg.spare);

  DictSafeSetItem(dict, "aton_type", msg.aton_type);
  DictSafeSetItem(dict, "name", msg.name);
  DictSafeSetItem(dict, "position_accuracy", msg.position_accuracy);
  DictSafeSetItem(dict, "x", "y", msg.position);
  DictSafeSetItem(dict, "dim_a", msg.dim_a);
  DictSafeSetItem(dict, "dim_b", msg.dim_b);
  DictSafeSetItem(dict, "dim_c", msg.dim_c);
  DictSafeSetItem(dict, "dim_d", msg.dim_d);
  DictSafeSetItem(dict, "fix_type", msg.fix_type);
  DictSafeSetItem(dict, "timestamp", msg.timestamp);
  DictSafeSetItem(dict, "off_pos", msg.off_pos);
  DictSafeSetItem(dict, "aton_status", msg.aton_status);
  DictSafeSetItem(dict, "raim", msg.raim);
  DictSafeSetItem(dict, "virtual_aton", msg.virtual_aton);
  DictSafeSetItem(dict, "assigned_mode", msg.assigned_mode);

  return dict;
}

// 22 - 'F' - Channel management
PyObject*
ais22_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  Ais22 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais22: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);
  DictSafeSetItem(dict, "spare", msg.spare);

  DictSafeSetItem(dict, "chan_a", msg.chan_a);
  DictSafeSetItem(dict, "chan_b", msg.chan_b);
  DictSafeSetItem(dict, "txrx_mode", msg.txrx_mode);
  DictSafeSetItem(dict, "power_low", msg.power_low);

  if (msg.pos_valid) {
    DictSafeSetItem(dict, "x1", "y1", msg.position1);
    DictSafeSetItem(dict, "x2", "y2", msg.position2);
  } else {
    DictSafeSetItem(dict, "dest_mmsi_1", msg.dest_mmsi_1);
    DictSafeSetItem(dict, "dest_mmsi_2", msg.dest_mmsi_2);
  }

  DictSafeSetItem(dict, "chan_a_bandwidth", msg.chan_a_bandwidth);
  DictSafeSetItem(dict, "chan_b_bandwidth", msg.chan_b_bandwidth);
  DictSafeSetItem(dict, "zone_size", msg.zone_size);
  DictSafeSetItem(dict, "spare2", msg.spare2);

  return dict;
}


// 23 - 'F' - Group assignment command
PyObject*
ais23_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  Ais23 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais23: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);
  DictSafeSetItem(dict, "spare", msg.spare);

  DictSafeSetItem(dict, "x1", "y1", msg.position1);
  DictSafeSetItem(dict, "x2", "y2", msg.position2);

  DictSafeSetItem(dict, "station_type", msg.station_type);
  DictSafeSetItem(dict, "type_and_cargo", msg.type_and_cargo);
  DictSafeSetItem(dict, "spare2", msg.spare2);

  DictSafeSetItem(dict, "txrx_mode", msg.txrx_mode);
  DictSafeSetItem(dict, "interval_raw", msg.interval_raw);
  DictSafeSetItem(dict, "quiet", msg.quiet);
  DictSafeSetItem(dict, "spare3", msg.spare3);

  return dict;
}

// 24 - 'H' - Static data report
PyObject*
ais24_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  Ais24 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais24: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  DictSafeSetItem(dict, "part_num", msg.part_num);

  switch (msg.part_num) {
  case 0:  // Part A
    DictSafeSetItem(dict, "name", msg.name);
    break;
  case 1:  // Part B
    DictSafeSetItem(dict, "type_and_cargo", msg.type_and_cargo);
    DictSafeSetItem(dict, "vendor_id", msg.vendor_id);
    DictSafeSetItem(dict, "callsign", msg.callsign);
    DictSafeSetItem(dict, "dim_a", msg.dim_a);
    DictSafeSetItem(dict, "dim_b", msg.dim_b);
    DictSafeSetItem(dict, "dim_c", msg.dim_c);
    DictSafeSetItem(dict, "dim_d", msg.dim_d);
    DictSafeSetItem(dict, "spare", msg.spare);
    break;
  case 2:  // FALLTHROUGH - not yet defined by ITU
  case 3:  // FALLTHROUGH - not yet defined by ITU
  default:
    // status = AIS_ERR_BAD_MSG_CONTENT;
    Py_DECREF(dict);
    PyErr_Format(ais_py_exception, "Ais24: unknown part_num %d",
                 msg.part_num);
    return nullptr;
  }

  return dict;
}

// 25 - 'I' - Single slot binary message
PyObject*
ais25_to_pydict(const char *nmea_payload, const size_t pad) {
  assert(nmea_payload);
  Ais25 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais25: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  // TODO(schwehr) use_app_id
  if (msg.dest_mmsi_valid) DictSafeSetItem(dict, "dest_mmsi", msg.dest_mmsi);
  if (msg.use_app_id) {
    DictSafeSetItem(dict, "dac", msg.dac);
    DictSafeSetItem(dict, "fi", msg.fi);
  }

  // TODO(schwehr): handle payload

  return dict;
}

// 26 - 'J' - Multi-slot binary message with commstate
PyObject*
ais26_to_pydict(const char *nmea_payload, const size_t pad) {
  Ais26 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais26: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  if (msg.dest_mmsi_valid) DictSafeSetItem(dict, "dest_mmsi", msg.dest_mmsi);
  if (msg.use_app_id) {
    DictSafeSetItem(dict, "dac", msg.dac);
    DictSafeSetItem(dict, "fi", msg.fi);
  }

  // TODO(schwehr): handle payload

  DictSafeSetItem(dict, "sync_state", msg.sync_state);
  if (0 == msg.commstate_flag) {
    // SOTDMA
    if (msg.received_stations_valid)
      DictSafeSetItem(dict, "received_stations", msg.received_stations);
    if (msg.slot_number_valid)
      DictSafeSetItem(dict, "slot_number", msg.slot_number);
    if (msg.utc_valid) {
      DictSafeSetItem(dict, "utc_hour", msg.utc_hour);
      DictSafeSetItem(dict, "utc_min", msg.utc_min);
      DictSafeSetItem(dict, "utc_spare", msg.utc_spare);
    }
    if (msg.slot_offset_valid)
      DictSafeSetItem(dict, "slot_offset", msg.slot_offset);
  } else {
    // ITDMA
    DictSafeSetItem(dict, "slot_increment", msg.slot_increment);
    DictSafeSetItem(dict, "slots_to_allocate", msg.slots_to_allocate);
    DictSafeSetItem(dict, "keep_flag", msg.keep_flag);
  }
  return dict;
}


// 27 - 'K' - Short position report for satellite reception
PyObject*
ais27_to_pydict(const char *nmea_payload, const size_t pad) {
  Ais27 msg(nmea_payload, pad);
  if (msg.had_error()) {
    PyErr_Format(ais_py_exception, "Ais27: %s",
                 AIS_STATUS_STRINGS[msg.get_error()]);
    return nullptr;
  }

  PyObject *dict = ais_msg_to_pydict(&msg);

  DictSafeSetItem(dict, "position_accuracy", msg.position_accuracy);
  DictSafeSetItem(dict, "raim", msg.raim);
  DictSafeSetItem(dict, "nav_status", msg.nav_status);
  DictSafeSetItem(dict, "x", "y", msg.position);
  DictSafeSetItem(dict, "sog", msg.sog);
  DictSafeSetItem(dict, "cog", msg.cog);
  DictSafeSetItem(dict, "gnss", msg.gnss);
  DictSafeSetItem(dict, "spare", msg.spare);
  return dict;
}


extern "C" {

static PyObject *
decode(PyObject *self, PyObject *args) {
  int _pad;
  const char *nmea_payload;
  // TODO(schwehr): what to do about if no pad bits?  Maybe warn and set to 0?
  if (!PyArg_ParseTuple(args, "si", &nmea_payload, &_pad)) {
    _pad = 0;
    if (!PyArg_ParseTuple(args, "s", &nmea_payload)) {
      PyErr_Format(ais_py_exception, "ais.decode: expected (str, int)");
      return nullptr;
    }
  }
  const size_t pad = _pad;

  // The grand dispatcher
  switch (nmea_payload[0]) {
  case '1':  // FALLTHROUGH - Class A Position
  case '2':  // FALLTHROUGH
  case '3':
    return ais1_2_3_to_pydict(nmea_payload, pad);

  case '4':  // FALLTHROUGH - 4 - Basestation report
  case ';':  // 11 - UTC date response
    return ais4_11_to_pydict(nmea_payload, pad);

  case '5':  // 5 - Ship and Cargo
    return ais5_to_pydict(nmea_payload, pad);

  case '6':  // 6 - Addressed binary message
    return ais6_to_pydict(nmea_payload, pad);

  case '7':  // FALLTHROUGH - 7 - ACK for addressed binary message
  case '=':  // 13 - ASRM Ack  (safety message)
    return ais7_13_to_pydict(nmea_payload, pad);

  case '8':  // 8 - Binary broadcast message (BBM)
    return ais8_to_pydict(nmea_payload, pad);

  case '9':  // 9 - SAR Position
    return ais9_to_pydict(nmea_payload, pad);

  case ':':  // 10 - UTC Query
    return ais10_to_pydict(nmea_payload, pad);

    // ';' 11 - See 4

  case '<':  // 12 - ASRM
    return ais12_to_pydict(nmea_payload, pad);

    // 13 - See 7

  case '>':  // 14 - SRBM - Safety broadcast
    return ais14_to_pydict(nmea_payload, pad);

  case '?':  // 15 - Interrogation
    return ais15_to_pydict(nmea_payload, pad);

  case '@':  // 16 - Assigned mode command
    return ais16_to_pydict(nmea_payload, pad);

  case 'A':  // 17 - GNSS broadcast
    return ais17_to_pydict(nmea_payload, pad);

  case 'B':  // 18 - Position, Class B
    return ais18_to_pydict(nmea_payload, pad);

  case 'C':  // 19 - Position and ship, Class B
    return ais19_to_pydict(nmea_payload, pad);

  case 'D':  // 20 - Data link management
    return ais20_to_pydict(nmea_payload, pad);

  case 'E':  // 21 - Aids to navigation report
    return ais21_to_pydict(nmea_payload, pad);

  case 'F':  // 22 - Channel Management
    return ais22_to_pydict(nmea_payload, pad);

  case 'G':  // 23 - Group Assignment Command
    return ais23_to_pydict(nmea_payload, pad);

  case 'H':  // 24 - Static data report
    return ais24_to_pydict(nmea_payload, pad);

  case 'I':  // 25 - Single slot binary message - addressed or broadcast
    // TODO(schwehr): handle payloads
    return ais25_to_pydict(nmea_payload, pad);

  case 'J':  // 26 - Multi slot binary message with comm state
    return ais26_to_pydict(nmea_payload, pad);  // TODO(schwehr): payloads

  case 'K':  // 27 - Long-range AIS broadcast message
    return ais27_to_pydict(nmea_payload, pad);

  case 'L':  // 28 - UNKNOWN
    PyErr_Format(ais_py_exception, "ais.decode: message 28 (L) not handled");
    break;

  default:
    PyErr_Format(ais_py_exception, "ais.decode: unknown message - %c",
                 nmea_payload[0]);
  }

  return nullptr;
}

static PyMethodDef ais_methods[] = {
  {"decode", decode, METH_VARARGS, "Return a dictionary for a NMEA string"},
  {nullptr, nullptr, 0, nullptr},  // Sentinel
};

// Python module initialization

static struct PyModuleDef aismodule = {
  PyModuleDef_HEAD_INIT,
  "_ais",  // Module name.
  nullptr,  // Module documentation.
  -1,  // Says the module keeps state in global variables.
  ais_methods
};

PyMODINIT_FUNC PyInit__ais(void) {
  PyObject *module = PyModule_Create(&aismodule);

  if (module == nullptr)
    return nullptr;

  ais_py_exception = PyErr_NewException(exception_name, nullptr, nullptr);
  Py_INCREF(ais_py_exception);
  PyModule_AddObject(module, exception_short, ais_py_exception);
  return module;
}

}  // extern "C"

}  // namespace libais
