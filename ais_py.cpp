
#include "ais.h"

#include <Python.h>
#include <cassert>
#include <iostream>
using namespace std;

extern "C" {

PyObject *
ais1_2_3_to_pydict(const char *nmea_payload) {
    assert(nmea_payload);
    assert(168/6 == strlen(nmea_payload));
    Ais1_2_3 msg(nmea_payload);

    PyObject *dict = PyDict_New();
    PyDict_SetItem(dict, PyString_FromString("id"), PyInt_FromLong(msg.message_id));
    PyDict_SetItem(dict, PyString_FromString("repeat_indicator"), PyInt_FromLong(msg.repeat_indicator));
    PyDict_SetItem(dict, PyString_FromString("mmsi"), PyInt_FromLong(msg.mmsi));

    PyDict_SetItem(dict, PyString_FromString("nav_status"), PyInt_FromLong(msg.nav_status));
    PyDict_SetItem(dict, PyString_FromString("rot_over_range"), PyBool_FromLong(msg.rot_over_range));
    PyDict_SetItem(dict, PyString_FromString("rot"), PyFloat_FromDouble(msg.rot));
    PyDict_SetItem(dict, PyString_FromString("sog"), PyFloat_FromDouble(msg.sog));
    PyDict_SetItem(dict, PyString_FromString("position_accuracy"), PyInt_FromLong(msg.position_accuracy));
    PyDict_SetItem(dict, PyString_FromString("x"), PyFloat_FromDouble(msg.x));
    PyDict_SetItem(dict, PyString_FromString("y"), PyFloat_FromDouble(msg.y));
    PyDict_SetItem(dict, PyString_FromString("cog"), PyFloat_FromDouble(msg.cog));
    PyDict_SetItem(dict, PyString_FromString("true_heading"), PyInt_FromLong(msg.true_heading));
    PyDict_SetItem(dict, PyString_FromString("timestamp"), PyInt_FromLong(msg.timestamp));
    PyDict_SetItem(dict, PyString_FromString("special_manoeuvre"), PyInt_FromLong(msg.special_manoeuvre));
    PyDict_SetItem(dict, PyString_FromString("spare"), PyInt_FromLong(msg.spare));
    PyDict_SetItem(dict, PyString_FromString("raim"), PyBool_FromLong(msg.raim));

    if (msg.received_stations_valid)
        PyDict_SetItem(dict, PyString_FromString("received_stations"), PyInt_FromLong(msg.received_stations));
    if (msg.slot_number_valid)
        PyDict_SetItem(dict, PyString_FromString("slot_number"), PyInt_FromLong(msg.slot_number));
    if (msg.utc_valid) {
        PyDict_SetItem(dict, PyString_FromString("utc_hour"), PyInt_FromLong(msg.utc_hour));
        PyDict_SetItem(dict, PyString_FromString("utc_min"), PyInt_FromLong(msg.utc_min));
        PyDict_SetItem(dict, PyString_FromString("utc_spare"), PyInt_FromLong(msg.utc_spare));
    }

    if (msg.slot_offset_valid)
        PyDict_SetItem(dict, PyString_FromString("slot_offset"), PyInt_FromLong(msg.slot_offset));

    if (msg.slot_increment_valid) {
        PyDict_SetItem(dict, PyString_FromString("slot_increment"), PyInt_FromLong(msg.slot_increment));
        PyDict_SetItem(dict, PyString_FromString("slots_to_allocate"), PyInt_FromLong(msg.slots_to_allocate));
        PyDict_SetItem(dict, PyString_FromString("keep_flag"), PyBool_FromLong(msg.keep_flag));
    }

    return dict;
}


PyObject * 
ais4_11_to_pydict(const char *nmea_payload) {
    assert(nmea_payload);
    assert(168/6 == strlen(nmea_payload));
    Ais4_11 msg(nmea_payload);

    PyObject *dict = PyDict_New();
    PyDict_SetItem(dict, PyString_FromString("id"), PyInt_FromLong(msg.message_id));
    PyDict_SetItem(dict, PyString_FromString("repeat_indicator"), PyInt_FromLong(msg.repeat_indicator));
    PyDict_SetItem(dict, PyString_FromString("mmsi"), PyInt_FromLong(msg.mmsi));

    PyDict_SetItem(dict, PyString_FromString("year"), PyInt_FromLong(msg.year));
    PyDict_SetItem(dict, PyString_FromString("month"), PyInt_FromLong(msg.month));
    PyDict_SetItem(dict, PyString_FromString("day"), PyInt_FromLong(msg.day));
    PyDict_SetItem(dict, PyString_FromString("hour"), PyInt_FromLong(msg.hour));
    PyDict_SetItem(dict, PyString_FromString("minute"), PyInt_FromLong(msg.minute));
    PyDict_SetItem(dict, PyString_FromString("second"), PyInt_FromLong(msg.second));

    PyDict_SetItem(dict, PyString_FromString("position_accuracy"), PyInt_FromLong(msg.position_accuracy));
    PyDict_SetItem(dict, PyString_FromString("x"), PyFloat_FromDouble(msg.x));
    PyDict_SetItem(dict, PyString_FromString("y"), PyFloat_FromDouble(msg.y));

    PyDict_SetItem(dict, PyString_FromString("fix_type"), PyInt_FromLong(msg.fix_type));
    PyDict_SetItem(dict, PyString_FromString("spare"), PyInt_FromLong(msg.spare));
    PyDict_SetItem(dict, PyString_FromString("raim"), PyBool_FromLong(msg.raim));


    if (msg.received_stations_valid)
        PyDict_SetItem(dict, PyString_FromString("received_stations"), PyInt_FromLong(msg.received_stations));
    if (msg.slot_number_valid)
        PyDict_SetItem(dict, PyString_FromString("slot_number"), PyInt_FromLong(msg.slot_number));
    if (msg.utc_valid) {
        PyDict_SetItem(dict, PyString_FromString("utc_hour"), PyInt_FromLong(msg.utc_hour));
        PyDict_SetItem(dict, PyString_FromString("utc_min"), PyInt_FromLong(msg.utc_min));
        PyDict_SetItem(dict, PyString_FromString("utc_spare"), PyInt_FromLong(msg.utc_spare));
    }

    if (msg.slot_offset_valid)
        PyDict_SetItem(dict, PyString_FromString("slot_offset"), PyInt_FromLong(msg.slot_offset));
    
    return dict;
}

PyObject * 
ais5_to_pydict(const char *nmea_payload) {
    assert(nmea_payload);
    assert((424+2)/6 == strlen(nmea_payload));
    Ais5 msg(nmea_payload);

    PyObject *dict = PyDict_New();
    PyDict_SetItem(dict, PyString_FromString("id"), PyInt_FromLong(msg.message_id));
    PyDict_SetItem(dict, PyString_FromString("repeat_indicator"), PyInt_FromLong(msg.repeat_indicator));
    PyDict_SetItem(dict, PyString_FromString("mmsi"), PyInt_FromLong(msg.mmsi));

    PyDict_SetItem(dict, PyString_FromString("ais_version"), PyInt_FromLong(msg.ais_version));
    PyDict_SetItem(dict, PyString_FromString("imo_num"), PyInt_FromLong(msg.imo_num));
    PyDict_SetItem(dict, PyString_FromString("callsign"), PyString_FromString(msg.callsign.c_str()));
    PyDict_SetItem(dict, PyString_FromString("name"), PyString_FromString(msg.name.c_str()));
    PyDict_SetItem(dict, PyString_FromString("type_and_cargo"), PyInt_FromLong(msg.type_and_cargo));
    PyDict_SetItem(dict, PyString_FromString("dim_a"), PyInt_FromLong(msg.dim_a));
    PyDict_SetItem(dict, PyString_FromString("dim_b"), PyInt_FromLong(msg.dim_b));
    PyDict_SetItem(dict, PyString_FromString("dim_c"), PyInt_FromLong(msg.dim_c));
    PyDict_SetItem(dict, PyString_FromString("dim_d"), PyInt_FromLong(msg.dim_d));
    PyDict_SetItem(dict, PyString_FromString("fix_type"), PyInt_FromLong(msg.fix_type));
    PyDict_SetItem(dict, PyString_FromString("eta_month"), PyInt_FromLong(msg.eta_month));
    PyDict_SetItem(dict, PyString_FromString("eta_day"), PyInt_FromLong(msg.eta_day));
    PyDict_SetItem(dict, PyString_FromString("eta_minute"), PyInt_FromLong(msg.eta_minute));
    PyDict_SetItem(dict, PyString_FromString("draught"), PyInt_FromLong(msg.draught));
    PyDict_SetItem(dict, PyString_FromString("destination"), PyString_FromString(msg.destination.c_str()));
    PyDict_SetItem(dict, PyString_FromString("dte"), PyInt_FromLong(msg.dte));
    PyDict_SetItem(dict, PyString_FromString("spare"), PyInt_FromLong(msg.spare));

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
    const size_t num_bits = strlen(nmea_payload) * 6;
    assert ( (40+32*1)==num_bits or (40+32*2)==num_bits or (40+32*3)==num_bits or (40+32*4) == num_bits);
    Ais7_13 msg(nmea_payload);
    
    PyObject *dict = PyDict_New();
    PyDict_SetItem(dict, PyString_FromString("id"), PyInt_FromLong(msg.message_id));
    PyDict_SetItem(dict, PyString_FromString("repeat_indicator"), PyInt_FromLong(msg.repeat_indicator));
    PyDict_SetItem(dict, PyString_FromString("mmsi"), PyInt_FromLong(msg.mmsi));

    PyObject *list = PyList_New(msg.dest_mmsi.size());
    for (size_t i=0; i < msg.dest_mmsi.size(); i++) {
        PyObject *tuple = PyTuple_New(2);
        PyTuple_SetItem(tuple,0,PyInt_FromLong(msg.dest_mmsi[i]));
        PyTuple_SetItem(tuple,1,PyInt_FromLong(msg.seq_num[i]));

        PyList_SetItem(list,i, tuple);
    }
    PyDict_SetItem(dict, PyString_FromString("acks"), list);

    return dict;
}


static PyObject *
decode(PyObject *self, PyObject *args) {

    if (!nmea_ord_initialized) {
        std::cout << "Calling build_nmea_lut from decode" << endl;
        build_nmea_lookup();
    }

    const char *nmea_payload;
    if (!PyArg_ParseTuple(args, "s", &nmea_payload))
        return NULL;
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
        assert (false);
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
        assert (false);
        break;
        
// 9 - SAR Position
    case '9':
        // result = ais9_to_pydict(nmea_payload);
        assert (false);
        break;

    // 10 - UTC Query
    case ':':
        //result = ais10_to_pydict(nmea_payload);
        assert (false);
        break;

        // 11 - See 4

    // 12 - ASRM
    case '<':
        // result = ais12_to_pydict(nmea_payload);
        assert (false);
        break;

        // 13 - See 7

    // 14 - SRBM
    case '>':
        // result = ais14_to_pydict(nmea_payload);
        assert (false);
        break;
    // 15 - Interrogation
    case '?':
        // result = ais15_to_pydict(nmea_payload);
        assert (false);
        break;
// 16 - Assigned mode command
    case '@':
        // result = ais16_to_pydict(nmea_payload);
        assert (false);
        break;
        
// 17 - GNSS broadcast
    case 'A':
        // result = ais17_to_pydict(nmea_payload);
        assert (false);
        break;
        
// 18 - Position, Class B
    case 'B':
        // result = ais18_to_pydict(nmea_payload);
        assert (false);
        break;
        
// 19 - Position and ship, Class B
    case 'C':
        // result = ais19_to_pydict(nmea_payload);
        assert (false);
        break;
        
// 20 - Data link management
    case 'D':
        // result = ais20_to_pydict(nmea_payload);
        assert (false);
        break;
        
// 21 - Aids to navigation report
    case 'E':
        // result = ais21_to_pydict(nmea_payload);
        assert (false);
        break;
        
// 22 - Channel Management
    case 'F':
        // result = ais22_to_pydict(nmea_payload);
        assert (false);
        break;
        
// 23 - Group Assignment Command
    case 'G':
        // result = ais23_to_pydict(nmea_payload);
        assert (false);
        break;
        
// 24 - Static data report
    case 'H':
        // result = ais24_to_pydict(nmea_payload);
        assert (false);
        break;
    
// 25 - Single slit binary message - addressed or broadcast
    case 'I':
        // result = ais25_to_pydict(nmea_payload);
        assert (false);
        break;
        
// 26 - Multi slot binary message with comm state
    case 'J':
        // result = ais26_to_pydict(nmea_payload);
        assert (false);
        break;
    
    default:
        assert (false);
    }

    assert (result);
    return result;
}       

static PyMethodDef ais_methods[] = {
    {"decode", decode, METH_VARARGS, "Return a dictionary for a NMEA string"},
    {NULL, NULL, 0, NULL},
};

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

}
