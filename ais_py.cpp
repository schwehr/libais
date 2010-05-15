
#include "ais.h"

#include <Python.h>
#include <cassert>
#include <iostream>
#include <string>
using namespace std;

PyObject *ais_py_exception;
const std::string exception_name("ais.decode.Error");

#if 0
// DO NOT do this
void TupleSafeSetItem(PyObject *tuple, const long position, const long val) {
    assert(tuple);
    PyObject *val_obj = PyLong_FromLong(val);
    assert(val_obj);
    PyTuple_SetItem(tuple,position, val_obj);
    Py_DECREF(val_obj);
}
#endif
    //inline
void
DictSafeSetItem(PyObject *dict, const std::string &key, const long val) {
    PyObject *key_obj = PyUnicode_FromString(key.c_str());
    PyObject *val_obj = PyLong_FromLong(val);
    assert(key_obj);
    assert(val_obj);
    PyDict_SetItem(dict, key_obj, val_obj);
    Py_DECREF(key_obj);
    Py_DECREF(val_obj);
}

void
DictSafeSetItem(PyObject *dict, const std::string &key, const int val) {
    PyObject *key_obj = PyUnicode_FromString(key.c_str());
    PyObject *val_obj = PyLong_FromLong(val);
    assert(key_obj);
    assert(val_obj);
    PyDict_SetItem(dict, key_obj, val_obj);
    Py_DECREF(key_obj);
    Py_DECREF(val_obj);
}

void
DictSafeSetItem(PyObject *dict, const std::string &key, const std::string &val) {
    PyObject *key_obj = PyUnicode_FromString(key.c_str());
    PyObject *val_obj = PyUnicode_FromString(val.c_str());
    assert(key_obj);
    assert(val_obj);
    PyDict_SetItem(dict, key_obj, val_obj);
    Py_DECREF(key_obj);
    Py_DECREF(val_obj);
}


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

// FIX: float -> double?
void
DictSafeSetItem(PyObject *dict, const std::string &key, const float val) {
    PyObject *key_obj = PyUnicode_FromString(key.c_str());
    PyObject *val_obj = PyFloat_FromDouble(val);
    assert(key_obj);
    assert(val_obj);
    PyDict_SetItem(dict, key_obj, val_obj);
    Py_DECREF(key_obj);
    Py_DECREF(val_obj);
}



extern "C" {

PyObject *
ais1_2_3_to_pydict(const char *nmea_payload) {
    assert(nmea_payload);

    Ais1_2_3 msg(nmea_payload);
    if (msg.had_error()) {
        PyErr_Format(ais_py_exception, "Ais1_2_3: %s", AIS_STATUS_STRINGS[msg.get_error()]);
        return 0;
    }

    PyObject *dict = PyDict_New();
    DictSafeSetItem(dict,"id", msg.message_id);
    DictSafeSetItem(dict,"repeat_indicator", msg.repeat_indicator);
    DictSafeSetItem(dict,"mmsi", msg.mmsi);

    DictSafeSetItem(dict,"nav_status", msg.nav_status);
    DictSafeSetItem(dict,"rot_over_range", msg.rot_over_range);
    DictSafeSetItem(dict,"rot",msg.rot);
    DictSafeSetItem(dict,"sog", msg.sog);
    DictSafeSetItem(dict,"position_accuracy", msg.position_accuracy);
    DictSafeSetItem(dict,"x", msg.x);
    DictSafeSetItem(dict,"y", msg.y);
    DictSafeSetItem(dict,"cog", msg.cog);
    DictSafeSetItem(dict,"true_heading", msg.true_heading);
    DictSafeSetItem(dict,"timestamp", msg.timestamp);
    DictSafeSetItem(dict,"special_manoeuvre", msg.special_manoeuvre);
    DictSafeSetItem(dict,"spare", msg.spare);
    DictSafeSetItem(dict,"raim", msg.raim);

    if (msg.received_stations_valid)
        DictSafeSetItem(dict,"received_stations", msg.received_stations);
    if (msg.slot_number_valid)
        DictSafeSetItem(dict,"slot_number", msg.slot_number);
    if (msg.utc_valid) {
        DictSafeSetItem(dict,"utc_hour", msg.utc_hour);
        DictSafeSetItem(dict,"utc_min", msg.utc_min);
        DictSafeSetItem(dict,"utc_spare", msg.utc_spare);
    }

    if (msg.slot_offset_valid)
        DictSafeSetItem(dict,"slot_offset", msg.slot_offset);

    if (msg.slot_increment_valid) {
        DictSafeSetItem(dict,"slot_increment", msg.slot_increment);
        DictSafeSetItem(dict,"slots_to_allocate", msg.slots_to_allocate);
        DictSafeSetItem(dict,"keep_flag", msg.keep_flag);
    }

    return dict;
}


PyObject * 
ais4_11_to_pydict(const char *nmea_payload) {
    assert(nmea_payload);
    //assert(168/6 == strlen(nmea_payload));  // Should be checked inside the class

    Ais4_11 msg(nmea_payload);
    if (msg.had_error()) {
        PyErr_Format(ais_py_exception, "Ais4_11: %s", AIS_STATUS_STRINGS[msg.get_error()]);
        return 0;
    }

    PyObject *dict = PyDict_New();
    DictSafeSetItem(dict,"id", msg.message_id);
    DictSafeSetItem(dict,"repeat_indicator", msg.repeat_indicator);
    DictSafeSetItem(dict,"mmsi", msg.mmsi);

    DictSafeSetItem(dict,"year", msg.year);
    DictSafeSetItem(dict,"month", msg.month);
    DictSafeSetItem(dict,"day", msg.day);
    DictSafeSetItem(dict,"hour", msg.hour);
    DictSafeSetItem(dict,"minute", msg.minute);
    DictSafeSetItem(dict,"second", msg.second);

    DictSafeSetItem(dict,"position_accuracy", msg.position_accuracy);
    DictSafeSetItem(dict,"x", msg.x);
    DictSafeSetItem(dict,"y", msg.y);

    DictSafeSetItem(dict,"fix_type", msg.fix_type);
    DictSafeSetItem(dict,"spare", msg.spare);
    DictSafeSetItem(dict,"raim", msg.raim);


    if (msg.received_stations_valid)
        DictSafeSetItem(dict,"received_stations", msg.received_stations);
    if (msg.slot_number_valid)
        DictSafeSetItem(dict,"slot_number", msg.slot_number);
    if (msg.utc_valid) {
        DictSafeSetItem(dict,"utc_hour", msg.utc_hour);
        DictSafeSetItem(dict,"utc_min", msg.utc_min);
        DictSafeSetItem(dict,"utc_spare", msg.utc_spare);
    }

    if (msg.slot_offset_valid)
        DictSafeSetItem(dict,"slot_offset", msg.slot_offset);
    
    return dict;
}



PyObject * 
ais5_to_pydict(const char *nmea_payload) {
    assert(nmea_payload);
    //assert((424+2)/6 == strlen(nmea_payload));  Check inside the class
    Ais5 msg(nmea_payload);
    if (msg.had_error()) {
        PyErr_Format(ais_py_exception, "Ais5: %s", AIS_STATUS_STRINGS[msg.get_error()]);
        return 0;
    }

    PyObject *dict = PyDict_New();

    DictSafeSetItem(dict,"id",msg.message_id);
    DictSafeSetItem(dict,"repeat_indicator", msg.repeat_indicator);
    DictSafeSetItem(dict,"mmsi", msg.mmsi);

    DictSafeSetItem(dict,"ais_version", msg.ais_version);
    DictSafeSetItem(dict,"imo_num", msg.imo_num);
    DictSafeSetItem(dict,"callsign", msg.callsign);
    DictSafeSetItem(dict,"name", msg.name);
    DictSafeSetItem(dict,"type_and_cargo", msg.type_and_cargo);
    DictSafeSetItem(dict,"dim_a", msg.dim_a);
    DictSafeSetItem(dict,"dim_b", msg.dim_b);
    DictSafeSetItem(dict,"dim_c", msg.dim_c);
    DictSafeSetItem(dict,"dim_d", msg.dim_d);
    DictSafeSetItem(dict,"fix_type", msg.fix_type);
    DictSafeSetItem(dict,"eta_month", msg.eta_month);
    DictSafeSetItem(dict,"eta_day", msg.eta_day);
    DictSafeSetItem(dict,"eta_minute", msg.eta_minute);
    DictSafeSetItem(dict,"draught", msg.draught);
    DictSafeSetItem(dict,"destination", msg.destination);
    DictSafeSetItem(dict,"dte", msg.dte);
    DictSafeSetItem(dict,"spare", msg.spare);

    return dict;
}

    /*
PyObject* 
ais6_to_pydict(const char *nmea_payload) {
    assert(false);
    }*/

PyObject* 
ais7_13_to_pydict(const char *nmea_payload) {
    assert(nmea_payload);
    //const size_t num_bits = strlen(nmea_payload) * 6;
    //assert ( (40+32*1)==num_bits or (40+32*2)==num_bits or (40+32*3)==num_bits or (40+32*4) == num_bits);
    // checked inside the message
    Ais7_13 msg(nmea_payload);
    if (msg.had_error()) {
        PyErr_Format(ais_py_exception, "Ais7_13: %s", AIS_STATUS_STRINGS[msg.get_error()]);
        return 0;
    }
    
    PyObject *dict = PyDict_New();
    DictSafeSetItem(dict,"id", msg.message_id);
    DictSafeSetItem(dict,"repeat_indicator", msg.repeat_indicator);
    DictSafeSetItem(dict,"mmsi", msg.mmsi);

    PyObject *list = PyList_New(msg.dest_mmsi.size());
    for (size_t i=0; i < msg.dest_mmsi.size(); i++) {
        // FIX: is this my memory leak?
        PyObject *tuple = PyTuple_New(2);
        PyTuple_SetItem(tuple,0,PyLong_FromLong(msg.dest_mmsi[i])); // Steals ref
        PyTuple_SetItem(tuple,1,PyLong_FromLong(msg.seq_num[i])); // Steals ref
        //TupleSafeSetItem(tuple, 0, msg.dest_mmsi[i]);
        //TupleSafeSetItem(tuple, 1, msg.seq_num[i]);

        PyList_SetItem(list,i, tuple); // Steals ref
        //Py_DECREF(list);
    }
    // FIX: probably a memory leak with list
    PyDict_SetItem(dict, PyUnicode_FromString("acks"), list);
    Py_DECREF(list);
    //Py_DECREF(list);
    //Py_DECREF(dict);
    return dict;
}

PyObject*
ais14_to_pydict(const char *nmea_payload) {
    assert(nmea_payload);
    Ais14 msg(nmea_payload);

    if (msg.had_error()) {
        PyErr_Format(ais_py_exception, "Ais14: %s", AIS_STATUS_STRINGS[msg.get_error()]);
        return 0;
    }

    PyObject *dict = PyDict_New();
    DictSafeSetItem(dict,"id", msg.message_id);
    DictSafeSetItem(dict,"repeat_indicator", msg.repeat_indicator);
    DictSafeSetItem(dict,"mmsi", msg.mmsi);
    DictSafeSetItem(dict,"text", msg.text);

    return dict;
}

PyObject*
ais18_to_pydict(const char *nmea_payload) {
    assert(nmea_payload);
    Ais18 msg(nmea_payload);
    if (msg.had_error()) {
        PyErr_Format(ais_py_exception, "Ais18: %s", AIS_STATUS_STRINGS[msg.get_error()]);
        return 0;
    }

    PyObject *dict = PyDict_New();
    DictSafeSetItem(dict,"id", msg.message_id);
    DictSafeSetItem(dict,"repeat_indicator", msg.repeat_indicator);
    DictSafeSetItem(dict,"mmsi", msg.mmsi);

    DictSafeSetItem(dict,"sog", msg.sog);
    DictSafeSetItem(dict,"position_accuracy", msg.position_accuracy);
    DictSafeSetItem(dict,"x", msg.x);
    DictSafeSetItem(dict,"y", msg.y);
    DictSafeSetItem(dict,"cog", msg.cog);
    DictSafeSetItem(dict,"true_heading", msg.true_heading);
    DictSafeSetItem(dict,"timestamp", msg.timestamp);

    DictSafeSetItem(dict,"spare2", msg.spare2);

    DictSafeSetItem(dict,"unit_flag", msg.unit_flag);
    DictSafeSetItem(dict,"display_flag", msg.display_flag);
    DictSafeSetItem(dict,"dsc_flag", msg.dsc_flag);
    DictSafeSetItem(dict,"band_flag", msg.band_flag);
    DictSafeSetItem(dict,"m22_flag", msg.m22_flag);
    DictSafeSetItem(dict,"mode_flag", msg.mode_flag);

    DictSafeSetItem(dict,"raim", msg.raim);

    DictSafeSetItem(dict,"commstate_flag", msg.commstate_flag);
    if (0==msg.unit_flag) {
        if (0==msg.commstate_flag) {
            // SOTDMA
            DictSafeSetItem(dict,"slot_timeout", msg.slot_timeout);
            switch (msg.slot_timeout) {
            case 0:
                DictSafeSetItem(dict,"slot_offset", msg.slot_offset);
                break;
            case 1:
                DictSafeSetItem(dict,"utc_hour", msg.utc_hour);
                DictSafeSetItem(dict,"utc_min", msg.utc_min);
                DictSafeSetItem(dict,"utc_spare", msg.utc_spare);
                break;
            case 2: // FALLTHROUGH
            case 4: // FALLTHROUGH
            case 6:
                DictSafeSetItem(dict,"slot_number", msg.slot_number);
                break;
            case 3: // FALLTHROUGH
            case 5: // FALLTHROUGH
            case 7:
                DictSafeSetItem(dict,"received_stations", msg.received_stations);
                break;
            default:
                std::cout << "ERROR: slot_timeout: " << msg.slot_timeout << std::endl;
                msg.print();
                assert(false); // Should never get here.
            }
            
        } else {
            // ITDMA
            DictSafeSetItem(dict,"slot_increment", msg.slot_increment);
            DictSafeSetItem(dict,"slots_to_allocate", msg.slots_to_allocate);
            DictSafeSetItem(dict,"keep_flag", msg.keep_flag);
        } 
    } // do nothing if unit flag is 1... in CS mode and no commstate
                   
    return dict;
}

PyObject*
ais19_to_pydict(const char *nmea_payload) {
    assert(nmea_payload);
    Ais19 msg(nmea_payload);
    if (msg.had_error()) {
        PyErr_Format(ais_py_exception, "Ais19: %s", AIS_STATUS_STRINGS[msg.get_error()]);
        return 0;
    }

    PyObject *dict = PyDict_New();
    DictSafeSetItem(dict,"id", msg.message_id);
    DictSafeSetItem(dict,"repeat_indicator", msg.repeat_indicator);
    DictSafeSetItem(dict,"mmsi", msg.mmsi);

    DictSafeSetItem(dict,"sog", msg.sog);
    DictSafeSetItem(dict,"position_accuracy", msg.position_accuracy);
    DictSafeSetItem(dict,"x", msg.x);
    DictSafeSetItem(dict,"y", msg.y);
    DictSafeSetItem(dict,"cog", msg.cog);
    DictSafeSetItem(dict,"true_heading", msg.true_heading);
    DictSafeSetItem(dict,"timestamp", msg.timestamp);

    DictSafeSetItem(dict,"spare2", msg.spare2);

    DictSafeSetItem(dict,"name", msg.name);
    DictSafeSetItem(dict,"type_and_cargo", msg.type_and_cargo);
    DictSafeSetItem(dict,"dim_a", msg.dim_a);
    DictSafeSetItem(dict,"dim_b", msg.dim_b);
    DictSafeSetItem(dict,"dim_c", msg.dim_c);
    DictSafeSetItem(dict,"dim_d", msg.dim_d);
    DictSafeSetItem(dict,"fix_type", msg.fix_type);

    DictSafeSetItem(dict,"raim", msg.raim);

    DictSafeSetItem(dict,"dte", msg.dte);
    DictSafeSetItem(dict,"assigned_mode", msg.assigned_mode);
    DictSafeSetItem(dict,"spare3", msg.spare3);

    return dict;
}

PyObject*
ais24_to_pydict(const char *nmea_payload) {
    assert(nmea_payload);
    Ais24 msg(nmea_payload);
    if (msg.had_error()) {
        PyErr_Format(ais_py_exception, "Ais24: %s", AIS_STATUS_STRINGS[msg.get_error()]);
        return 0;
    }

    PyObject *dict = PyDict_New();
    DictSafeSetItem(dict,"id", msg.message_id);
    DictSafeSetItem(dict,"repeat_indicator", msg.repeat_indicator);
    DictSafeSetItem(dict,"mmsi", msg.mmsi);

    DictSafeSetItem(dict,"part_num", msg.part_num);

    switch(msg.part_num) {

    case 0: // Part A
        DictSafeSetItem(dict,"name",msg.name);
        break;

    case 1: // Part B
        DictSafeSetItem(dict,"type_and_cargo",msg.type_and_cargo);
        DictSafeSetItem(dict,"vendor_id",msg.vendor_id);
        DictSafeSetItem(dict,"callsign",msg.callsign);
        DictSafeSetItem(dict,"dim_a",msg.dim_a);
        DictSafeSetItem(dict,"dim_b",msg.dim_b);
        DictSafeSetItem(dict,"dim_c",msg.dim_c);
        DictSafeSetItem(dict,"dim_d",msg.dim_d);
        DictSafeSetItem(dict,"spare",msg.spare);
        break;

    case 2: // FALLTHROUGH - not yet defined by ITU
    case 3: // FALLTHROUGH - not yet defined by ITU
    default:
        // status = AIS_ERR_BAD_MSG_CONTENT;
        // FIX: setup python exception
        return 0;
    }


    return dict;
}


static PyObject *
decode(PyObject *self, PyObject *args) {

    if (!nmea_ord_initialized) {
        std::cout << "Calling build_nmea_lut from decode" << endl;
        build_nmea_lookup();
    }

    const char *nmea_payload;
    if (!PyArg_ParseTuple(args, "s", &nmea_payload)) {
        PyErr_Format(ais_py_exception, "ais.decode: expected string argument");
        return 0;
    }
    //cout << "nmea_payload: " << nmea_payload << endl;

    PyObject *result=0;

    //
    // The grand dispatcher
    //
    switch (nmea_payload[0]) {

    // Class A Position
    case '1': // FALLTHROUGH
    case '2': // FALLTHROUGH
    case '3':
        result = ais1_2_3_to_pydict(nmea_payload);
        break;

        // 4 - Basestation report
        // 11 - UTC date response
    case '4': // FALLTHROUGH 
    case ';':
        result = ais4_11_to_pydict(nmea_payload);
        break;

    case '5': // 5 - Ship and Cargo
        result = ais5_to_pydict(nmea_payload);
        break;
        
    case '6': // 6 - Addressed binary message
        // result = ais6_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 6 not yet handled");
        break;
        
        // 7 - ACK for addressed binary message
        // 13 - ASRM Ack  (safety message)
    case '=': // FALLTHROUGH
    case '7':
        //std::cerr << "7_or_14: " << nmea_payload << std::endl;
        result = ais7_13_to_pydict(nmea_payload);
        break;
        
    case '8': // 8 - Binary broadcast message (BBM)
        // result = ais8_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 8 not yet handled");
        break;
        
    case '9': // 9 - SAR Position
        // result = ais9_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 9 not yet handled");
        break;

    case ':': // 10 - UTC Query
        //result = ais10_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 10 (;) not yet handled");
        break;

        // ':' 11 - See 4
   
    case '<': // 12 - ASRM
        // result = ais12_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 12 (<) not yet handled");
        break;

        // 13 - See 7

    case '>': // 14 - SRBM - Safety broadcast
        result = ais14_to_pydict(nmea_payload);
        break;

    case '?': // 15 - Interrogation
        // result = ais15_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 15 (?) not yet handled");
        break;

    case '@': // 16 - Assigned mode command
        // result = ais16_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 16 (@) not yet handled");
        break;
        
    case 'A': // 17 - GNSS broadcast
        // result = ais17_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 17 (A) not yet handled");
        break;
        
    case 'B': // 18 - Position, Class B
        result = ais18_to_pydict(nmea_payload);
        break;
        
    case 'C': // 19 - Position and ship, Class B
        result = ais19_to_pydict(nmea_payload);
        break;
        
    case 'D': // 20 - Data link management
        // result = ais20_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 20 (D) not yet handled");
        break;
        
    case 'E': // 21 - Aids to navigation report
        // result = ais21_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 21 (E) not yet handled");
        break;
        
    case 'F': // 22 - Channel Management
        // result = ais22_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 22 (F) not yet handled");
        break;
        
    case 'G': // 23 - Group Assignment Command
        // result = ais23_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 23 (G) not yet handled");
        break;
        
    case 'H': // 24 - Static data report
        result = ais24_to_pydict(nmea_payload);
        break;
    
    case 'I': // 25 - Single slot binary message - addressed or broadcast
        // result = ais25_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 25 (I) not yet handled");
        break;
        
    case 'J': // 26 - Multi slot binary message with comm state
        // result = ais26_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 26 (J) not yet handled");
        break;

        // 27 - K
        // 28 - L
    
    default:
        //assert (false);
        //std::cout << "Unknown message type: '" << nmea_payload[0] << "'\n"
        //          << "\tline: " << nmea_payload << std::endl;
        PyErr_Format(ais_py_exception, "ais.decode: message %c not known", nmea_payload[0]);

    }


    return result;
}       

static PyMethodDef ais_methods[] = {
    {"decode", decode, METH_VARARGS, "Return a dictionary for a NMEA string"},
    {NULL, NULL, 0, NULL},
};

#if 0
PyMODINIT_FUNC
initais(void) 
{
    PyObject *mod;
    mod = Py_InitModule("ais", ais_methods);
    if (mod == NULL) {
        std::cerr << "UNABLE TO LOAD MODULE";
        return;
    }
    build_nmea_lookup();
    std::cerr << "built lookup table in initais" << std::endl;
}
#else
//////////////////////////////////////////////////////////////////////
// Python 3 initialization
//////////////////////////////////////////////////////////////////////

struct module_state {
    PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

#if 0
static PyObject *
error_out(PyObject *m) {
    struct module_state *st = GETSTATE(m);
    PyErr_SetString(st->error, "something bad happened");
    return NULL;
}

static PyMethodDef ais_methods[] = {
    {"error_out", (PyCFunction)error_out, METH_NOARGS, NULL},
    {NULL, NULL}
};
#endif

#if PY_MAJOR_VERSION >= 3

static int ais_traverse(PyObject *m, visitproc visit, void *arg) {
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}

static int ais_clear(PyObject *m) {
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}


static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "ais",
        NULL,
        sizeof(struct module_state),
        ais_methods,
        NULL,
        ais_traverse,
        ais_clear,
        NULL
};

#define INITERROR return NULL

PyObject *
PyInit_ais(void)

#warning "Should be okay"

#else
#define INITERROR return

void
initais(void)
#endif
{
#if PY_MAJOR_VERSION >= 3
    PyObject *module = PyModule_Create(&moduledef);
#else
    PyObject *module = Py_InitModule("ais", ais_methods);
#endif

    if (module == NULL)
        INITERROR;
    struct module_state *st = GETSTATE(module);


    st->error = PyErr_NewException((char *)exception_name.c_str(), NULL, NULL);
    if (st->error == NULL) {
        Py_DECREF(module);
        INITERROR;
    }

    // Initialize the lookuptable and exception
    build_nmea_lookup();
    ais_py_exception = PyErr_NewException("ais.decode.error", 0, 0);

#if PY_MAJOR_VERSION >= 3
    return module;
#endif
}



#endif


} // extern "C"
