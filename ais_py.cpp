
#include "ais.h"

#include <Python.h>
#include <cassert>
#include <iostream>
using namespace std;

PyObject *ais_py_exception;


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
    PyDict_SetItem(dict, PyUnicode_FromString("id"), PyLong_FromLong(msg.message_id));
    PyDict_SetItem(dict, PyUnicode_FromString("repeat_indicator"), PyLong_FromLong(msg.repeat_indicator));
    PyDict_SetItem(dict, PyUnicode_FromString("mmsi"), PyLong_FromLong(msg.mmsi));

    PyDict_SetItem(dict, PyUnicode_FromString("nav_status"), PyLong_FromLong(msg.nav_status));
    PyDict_SetItem(dict, PyUnicode_FromString("rot_over_range"), PyBool_FromLong(msg.rot_over_range));
    PyDict_SetItem(dict, PyUnicode_FromString("rot"), PyFloat_FromDouble(msg.rot));
    PyDict_SetItem(dict, PyUnicode_FromString("sog"), PyFloat_FromDouble(msg.sog));
    PyDict_SetItem(dict, PyUnicode_FromString("position_accuracy"), PyLong_FromLong(msg.position_accuracy));
    PyDict_SetItem(dict, PyUnicode_FromString("x"), PyFloat_FromDouble(msg.x));
    PyDict_SetItem(dict, PyUnicode_FromString("y"), PyFloat_FromDouble(msg.y));
    PyDict_SetItem(dict, PyUnicode_FromString("cog"), PyFloat_FromDouble(msg.cog));
    PyDict_SetItem(dict, PyUnicode_FromString("true_heading"), PyLong_FromLong(msg.true_heading));
    PyDict_SetItem(dict, PyUnicode_FromString("timestamp"), PyLong_FromLong(msg.timestamp));
    PyDict_SetItem(dict, PyUnicode_FromString("special_manoeuvre"), PyLong_FromLong(msg.special_manoeuvre));
    PyDict_SetItem(dict, PyUnicode_FromString("spare"), PyLong_FromLong(msg.spare));
    PyDict_SetItem(dict, PyUnicode_FromString("raim"), PyBool_FromLong(msg.raim));

    if (msg.received_stations_valid)
        PyDict_SetItem(dict, PyUnicode_FromString("received_stations"), PyLong_FromLong(msg.received_stations));
    if (msg.slot_number_valid)
        PyDict_SetItem(dict, PyUnicode_FromString("slot_number"), PyLong_FromLong(msg.slot_number));
    if (msg.utc_valid) {
        PyDict_SetItem(dict, PyUnicode_FromString("utc_hour"), PyLong_FromLong(msg.utc_hour));
        PyDict_SetItem(dict, PyUnicode_FromString("utc_min"), PyLong_FromLong(msg.utc_min));
        PyDict_SetItem(dict, PyUnicode_FromString("utc_spare"), PyLong_FromLong(msg.utc_spare));
    }

    if (msg.slot_offset_valid)
        PyDict_SetItem(dict, PyUnicode_FromString("slot_offset"), PyLong_FromLong(msg.slot_offset));

    if (msg.slot_increment_valid) {
        PyDict_SetItem(dict, PyUnicode_FromString("slot_increment"), PyLong_FromLong(msg.slot_increment));
        PyDict_SetItem(dict, PyUnicode_FromString("slots_to_allocate"), PyLong_FromLong(msg.slots_to_allocate));
        PyDict_SetItem(dict, PyUnicode_FromString("keep_flag"), PyBool_FromLong(msg.keep_flag));
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
    PyDict_SetItem(dict, PyUnicode_FromString("id"), PyLong_FromLong(msg.message_id));
    PyDict_SetItem(dict, PyUnicode_FromString("repeat_indicator"), PyLong_FromLong(msg.repeat_indicator));
    PyDict_SetItem(dict, PyUnicode_FromString("mmsi"), PyLong_FromLong(msg.mmsi));

    PyDict_SetItem(dict, PyUnicode_FromString("year"), PyLong_FromLong(msg.year));
    PyDict_SetItem(dict, PyUnicode_FromString("month"), PyLong_FromLong(msg.month));
    PyDict_SetItem(dict, PyUnicode_FromString("day"), PyLong_FromLong(msg.day));
    PyDict_SetItem(dict, PyUnicode_FromString("hour"), PyLong_FromLong(msg.hour));
    PyDict_SetItem(dict, PyUnicode_FromString("minute"), PyLong_FromLong(msg.minute));
    PyDict_SetItem(dict, PyUnicode_FromString("second"), PyLong_FromLong(msg.second));

    PyDict_SetItem(dict, PyUnicode_FromString("position_accuracy"), PyLong_FromLong(msg.position_accuracy));
    PyDict_SetItem(dict, PyUnicode_FromString("x"), PyFloat_FromDouble(msg.x));
    PyDict_SetItem(dict, PyUnicode_FromString("y"), PyFloat_FromDouble(msg.y));

    PyDict_SetItem(dict, PyUnicode_FromString("fix_type"), PyLong_FromLong(msg.fix_type));
    PyDict_SetItem(dict, PyUnicode_FromString("spare"), PyLong_FromLong(msg.spare));
    PyDict_SetItem(dict, PyUnicode_FromString("raim"), PyBool_FromLong(msg.raim));


    if (msg.received_stations_valid)
        PyDict_SetItem(dict, PyUnicode_FromString("received_stations"), PyLong_FromLong(msg.received_stations));
    if (msg.slot_number_valid)
        PyDict_SetItem(dict, PyUnicode_FromString("slot_number"), PyLong_FromLong(msg.slot_number));
    if (msg.utc_valid) {
        PyDict_SetItem(dict, PyUnicode_FromString("utc_hour"), PyLong_FromLong(msg.utc_hour));
        PyDict_SetItem(dict, PyUnicode_FromString("utc_min"), PyLong_FromLong(msg.utc_min));
        PyDict_SetItem(dict, PyUnicode_FromString("utc_spare"), PyLong_FromLong(msg.utc_spare));
    }

    if (msg.slot_offset_valid)
        PyDict_SetItem(dict, PyUnicode_FromString("slot_offset"), PyLong_FromLong(msg.slot_offset));
    
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
    PyDict_SetItem(dict, PyUnicode_FromString("id"), PyLong_FromLong(msg.message_id));
    PyDict_SetItem(dict, PyUnicode_FromString("repeat_indicator"), PyLong_FromLong(msg.repeat_indicator));
    PyDict_SetItem(dict, PyUnicode_FromString("mmsi"), PyLong_FromLong(msg.mmsi));

    PyDict_SetItem(dict, PyUnicode_FromString("ais_version"), PyLong_FromLong(msg.ais_version));
    PyDict_SetItem(dict, PyUnicode_FromString("imo_num"), PyLong_FromLong(msg.imo_num));
    PyDict_SetItem(dict, PyUnicode_FromString("callsign"), PyUnicode_FromString(msg.callsign.c_str()));
    PyDict_SetItem(dict, PyUnicode_FromString("name"), PyUnicode_FromString(msg.name.c_str()));
    PyDict_SetItem(dict, PyUnicode_FromString("type_and_cargo"), PyLong_FromLong(msg.type_and_cargo));
    PyDict_SetItem(dict, PyUnicode_FromString("dim_a"), PyLong_FromLong(msg.dim_a));
    PyDict_SetItem(dict, PyUnicode_FromString("dim_b"), PyLong_FromLong(msg.dim_b));
    PyDict_SetItem(dict, PyUnicode_FromString("dim_c"), PyLong_FromLong(msg.dim_c));
    PyDict_SetItem(dict, PyUnicode_FromString("dim_d"), PyLong_FromLong(msg.dim_d));
    PyDict_SetItem(dict, PyUnicode_FromString("fix_type"), PyLong_FromLong(msg.fix_type));
    PyDict_SetItem(dict, PyUnicode_FromString("eta_month"), PyLong_FromLong(msg.eta_month));
    PyDict_SetItem(dict, PyUnicode_FromString("eta_day"), PyLong_FromLong(msg.eta_day));
    PyDict_SetItem(dict, PyUnicode_FromString("eta_minute"), PyLong_FromLong(msg.eta_minute));
    PyDict_SetItem(dict, PyUnicode_FromString("draught"), PyLong_FromLong(msg.draught));
    PyDict_SetItem(dict, PyUnicode_FromString("destination"), PyUnicode_FromString(msg.destination.c_str()));
    PyDict_SetItem(dict, PyUnicode_FromString("dte"), PyLong_FromLong(msg.dte));
    PyDict_SetItem(dict, PyUnicode_FromString("spare"), PyLong_FromLong(msg.spare));

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
    PyDict_SetItem(dict, PyUnicode_FromString("id"), PyLong_FromLong(msg.message_id));
    PyDict_SetItem(dict, PyUnicode_FromString("repeat_indicator"), PyLong_FromLong(msg.repeat_indicator));
    PyDict_SetItem(dict, PyUnicode_FromString("mmsi"), PyLong_FromLong(msg.mmsi));

    PyObject *list = PyList_New(msg.dest_mmsi.size());
    for (size_t i=0; i < msg.dest_mmsi.size(); i++) {
        PyObject *tuple = PyTuple_New(2);
        PyTuple_SetItem(tuple,0,PyLong_FromLong(msg.dest_mmsi[i]));
        PyTuple_SetItem(tuple,1,PyLong_FromLong(msg.seq_num[i]));

        PyList_SetItem(list,i, tuple);
    }
    PyDict_SetItem(dict, PyUnicode_FromString("acks"), list);

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
    cout << "nmea_payload: " << nmea_payload << endl;

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

// 5 - Ship and Cargo
    case '5':
        result = ais5_to_pydict(nmea_payload);
        break;
        
// 6 - Addressed binary message
    case '6':
        // result = ais6_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 6 not yet handled");
        break;
        
// 7 - ACK for addressed binary message
// 13 - ASRM Ack  (safety message)
    case '=': // FALLTHROUGH
    case '7':
        result = ais7_13_to_pydict(nmea_payload);
        break;
        
// 8 - Binary broadcast message (BBM)
    case '8':
        // result = ais8_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 8 not yet handled");
        break;
        
// 9 - SAR Position
    case '9':
        // result = ais9_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 9 not yet handled");
        break;

    // 10 - UTC Query
    case ':':
        //result = ais10_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 10 (;) not yet handled");
        break;

        // 11 - See 4

    // 12 - ASRM
    case '<':
        // result = ais12_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 12 (<) not yet handled");
        break;

        // 13 - See 7

    // 14 - SRBM - Safety broadcast
    case '>':
        // result = ais14_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 14 (>) not yet handled");
        break;
    // 15 - Interrogation
    case '?':
        // result = ais15_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 15 (?) not yet handled");
        break;
// 16 - Assigned mode command
    case '@':
        // result = ais16_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 16 (@) not yet handled");
        break;
        
// 17 - GNSS broadcast
    case 'A':
        // result = ais17_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 17 (A) not yet handled");
        break;
        
// 18 - Position, Class B
    case 'B':
        result = ais18_to_pydict(nmea_payload);
        //PyErr_Format(ais_py_exception, "ais.decode: message 18 (B) not yet handled");
        break;
        
// 19 - Position and ship, Class B
    case 'C':
        // result = ais19_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 19 (C) not yet handled");
        break;
        
// 20 - Data link management
    case 'D':
        // result = ais20_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 20 (D) not yet handled");
        break;
        
// 21 - Aids to navigation report
    case 'E':
        // result = ais21_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 21 (E) not yet handled");
        break;
        
// 22 - Channel Management
    case 'F':
        // result = ais22_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 22 (F) not yet handled");
        break;
        
// 23 - Group Assignment Command
    case 'G':
        // result = ais23_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 23 (G) not yet handled");
        break;
        
// 24 - Static data report
    case 'H':
        // result = ais24_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 24 (H) not yet handled");
        break;
    
// 25 - Single slit binary message - addressed or broadcast
    case 'I':
        // result = ais25_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 25 (I) not yet handled");
        break;
        
// 26 - Multi slot binary message with comm state
    case 'J':
        // result = ais26_to_pydict(nmea_payload);
        PyErr_Format(ais_py_exception, "ais.decode: message 26 (J) not yet handled");
        break;
    
    default:
        assert (false);
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

    st->error = PyErr_NewException("ais.Error", NULL, NULL);
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
