#include <Python.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

static PyObject* communicate(PyObject* self, PyObject* args) {
    struct sockaddr_rc addr = { 0 };
    char *bssid;
    int sz, sz2;
    int channel, sock, stat;
    Py_buffer atcmd;
    char *recv_buffer;
    PyObject *ret;

    ret = PyBytes_FromStringAndSize("", 0);

    memset(bssid, 0, 1024);
    if (!PyArg_ParseTuple(args, "siiy*", &bssid, &channel, &sz, &atcmd)) {
        PyErr_SetString(PyExc_ValueError, "bad params.");
        return ret;
    }

    if (sz <= 0) {
        PyErr_SetString(PyExc_ValueError, "wrong size.");
        return ret;
    }

    recv_buffer = (char *)malloc(sz);
    if (recv_buffer <= 0) {
         PyErr_NoMemory();
         return ret;
    }

    memset(recv_buffer, 0, sz);

    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sock < 0) {
        free(recv_buffer);
        PyBuffer_Release(&atcmd);
        PyErr_SetString(PyExc_ValueError, "bluetooth interface not found.");
        return ret;
    }

    addr.rc_family = AF_BLUETOOTH;
    str2ba(bssid, &addr.rc_bdaddr);
    addr.rc_channel = (uint8_t)channel;
    stat = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (stat < 0) {
        close(sock);
        free(recv_buffer);
        PyBuffer_Release(&atcmd);
        PyErr_SetString(PyExc_ValueError, "cannot connect.");
        return ret;
    }

    sz2 = write(sock, (char *)atcmd.buf, (size_t)atcmd.len); 
    if (sz2 < atcmd.len) {
        close(sock);
        free(recv_buffer);
        PyBuffer_Release(&atcmd);
        PyErr_SetString(PyExc_ValueError, "it was not possible to send the data totally.");
        return ret;
    }

    sz2 = recv(sock, recv_buffer, sz, 0);
    close(sock);

    ret = PyBytes_FromStringAndSize(recv_buffer, sz2);

    free(recv_buffer);
    PyBuffer_Release(&atcmd);
    return ret;
}

static PyMethodDef ModMethods[] = {
    {"communicate", communicate, METH_VARARGS, "Launch raw communications over an RFCOMM channel."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef RFCommRAW = {
    PyModuleDef_HEAD_INIT,
    "RFCommRAW",  
    NULL, // documentation
    -1,   // stat size
    ModMethods
};


PyMODINIT_FUNC PyInit_RFCommRAW(void) {
    PyObject *m;

    m = PyModule_Create(&RFCommRAW);
    if (m == NULL) {
        return NULL;
    }

    return m;
}


PyMODINIT_FUNC PyInit_modmodule(void) {
    return PyModule_Create(&RFCommRAW);
}

