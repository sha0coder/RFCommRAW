#include <Python.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
//#include <bluetooth/l2cap.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>


static PyObject *interfaces(PyObject *self, PyObject *args) {
    int sock, dev_id;
    PyObject *list = PyList_New(0);

    if (!list) {
        PyErr_SetString(PyExc_MemoryError, "low memory");
        return NULL;
    }

    for (dev_id=0; dev_id<0xffff; dev_id++) {
        sock = hci_open_dev(dev_id);
        if (sock >= 0) {
            PyObject *id = PyLong_FromLong(dev_id);
            PyList_Append(list, id);
            Py_DECREF(id);
            close(sock);
        }
    }

    return list;
}

static PyObject *scan(PyObject *self, PyObject *args) {
    inquiry_info *ii = NULL;
    int dev_id, i;
    int max_rsp, num_rsp;
    int sock, len, flags;
    char addr[19] = { 0 };
    char name[248] = { 0 };
    PyObject *list = PyList_New(0);

    if (!list) {
        PyErr_SetString(PyExc_MemoryError, "low memory");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "i", &dev_id)) {
        PyErr_SetString(PyExc_ValueError, "bad params.");
        return NULL;
    }

    sock = hci_open_dev( dev_id );
    if (dev_id < 0 || sock < 0) {
        PyErr_SetString(PyExc_ValueError, "wrong device.");
        return NULL;
    }

    len  = 8;
    max_rsp = 255;
    flags = IREQ_CACHE_FLUSH;
    ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));
    if (ii == NULL) {
        PyErr_SetString(PyExc_MemoryError, "low memory");
        return NULL;
    }

    num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
    if ( num_rsp < 0 ) {
        PyErr_SetString(PyExc_ValueError, "hci inquiry error");
        return NULL;
    }

    for (i = 0; i < num_rsp; i++) {
        ba2str(&(ii+i)->bdaddr, addr);
        memset(name, 0, sizeof(name));

        if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0) == 0) {
            PyObject *s = PyUnicode_FromString(addr);
            PyList_Append(list, s);
            Py_DECREF(s);
        }
        //    strcpy(name, "[unknown]");
        //printf(" %s  %s rep:%u period:%u mode:%u class:%u.%u.%u clock:%u\n",
        //addr, name, *&(ii+i)->pscan_rep_mode, *&(ii+i)->pscan_period_mode, *&(ii+i)->pscan_mode,
        //*&(ii+i)->dev_class[0], *&(ii+i)->dev_class[1], *&(ii+i)->dev_class[2]), *&(ii+i)->clock_offset;
    }

    free( ii );
    close( sock );

    return list;
}

/*
static PyObject *lescan(PyObject *self, PyObject *args) {
    int sock, dev_id;
    size_t n;
    char buff[261];
    char opts[16];
    struct iovec iov[3];
    struct sockaddr_hci hci;
    PyObject *list = PyList_New(0);

    if (!list) {
        PyErr_SetString(PyExc_MemoryError, "low memory");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "i", &dev_id)) {
        PyErr_SetString(PyExc_ValueError, "bad params.");
        return NULL;
    }

    hci.hci_family = AF_BLUETOOTH;
    hci.hci_dev = htobs(dev_id);
    hci.hci_channel = HCI_CHANNEL_RAW;

    sock = socket(AF_BLUETOOTH, SOCK_RAW|SOCK_CLOEXEC, BTPROTO_HCI);
    if (sock < 0) {
        PyErr_SetString(PyExc_ValueError, "socket error");
        return NULL;
    }

    n = bind(sock, (struct sockaddr *)&hci, 6);
    if (n < 0) {
        PyErr_SetString(PyExc_ValueError, "bind error");
        return NULL;
    }

    iov[0].iov_base = "\1";
    iov[0].iov_len = 1;
    iov[1].iov_base = "\v \7";
    iov[1].iov_len = 3;
    iov[2].iov_base = "\1\20\0\20\0\0\0";
    iov[2].iov_len = 7;

    strncpy(opts, "\20\0\0\0\1\300\0\0\0\0\0@\v \0\0", 16);
    setsockopt(sock, SOL_IP, IP_TTL, &opts, 16);
    n = writev(sock, (struct iovec *)&iov, 3);
    if (n < 0) {
        PyErr_SetString(PyExc_ValueError, "writev error");
        return NULL;
    }

    n = read(sock, buff, 260);
    if (n < 0) {
        PyErr_SetString(PyExc_ValueError, "read error");
        return NULL;
    }

    iov[0].iov_base = "\1";
    iov[0].iov_len = 1;
    iov[1].iov_base = "\f \2";
    iov[1].iov_len = 3;
    iov[2].iov_base = "\1\1";
    iov[2].iov_len = 2;

    strncpy(opts, "\20\0\0\0\1\300\0\0\0\0\0@\f \0\0", 16);
    setsockopt(sock, SOL_IP, IP_TTL, &opts, 16);
    n = writev(sock, (struct iovec *)&iov, 3);
    if (n < 0) {
        PyErr_SetString(PyExc_ValueError, "writev error");
        return NULL;
    }

    buff[260] = 0x00; 
    n = read(sock, buff, 260);
    if (n < 0) {
        PyErr_SetString(PyExc_ValueError, "read error");
        return NULL;
    }

    strncpy(opts, "\20\0\0\0\0\0\0\0\0\0\0@\0\0\0\0", 16);
    setsockopt(sock, SOL_IP, IP_TTL, &opts, 16);

    while (1) {
        memset(buff, 0, 260);
        n = read(sock, buff, 260);
        if (n < 0) {
            PyErr_SetString(PyExc_ValueError, "read error");
            return NULL;
        }

        PyObject *s = PyUnicode_FromString(buff);
        PyList_Append(list, s);
        Py_DECREF(s);
    }

    return list;
}*/

static PyObject *communicate(PyObject *self, PyObject *args) {
    struct sockaddr_rc addr = { 0 };
    char *bssid;
    int sz, sz2, timeout, result;
    int channel, sock, stat, flags;
    Py_buffer atcmd;
    char *recv_buffer;
    PyObject *ret;
    fd_set write_fds, read_fds;
    struct timeval tv;

    if (!PyArg_ParseTuple(args, "siiiy*", &bssid, &channel, &sz, &timeout, &atcmd)) {
        PyErr_SetString(PyExc_ValueError, "bad params.");
        return NULL;
    }

    if (sz <= 0) {
        PyErr_SetString(PyExc_ValueError, "wrong size.");
        return NULL;
    }

    recv_buffer = (char *)malloc(sz);
    if (recv_buffer <= 0) {
         PyErr_NoMemory();
         return NULL;
    }

    memset(recv_buffer, 0, sz);

    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sock < 0) {
        free(recv_buffer);
        PyBuffer_Release(&atcmd);
        PyErr_SetString(PyExc_ValueError, "bluetooth interface not found.");
        return NULL;
    }

    addr.rc_family = AF_BLUETOOTH;
    str2ba(bssid, &addr.rc_bdaddr);
    addr.rc_channel = (uint8_t)channel;

    flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    stat = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (stat < 0) {
        if (errno != EINPROGRESS) {
            close(sock);
            free(recv_buffer);
            PyBuffer_Release(&atcmd);
            PyErr_SetString(PyExc_ValueError, "cannot connect.");
            return NULL;
        }
    }


    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_SET(sock, &read_fds);
    FD_SET(sock, &write_fds);
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    while (1) {
        result = select(sock + 1, &read_fds, &write_fds, NULL, &tv);
        if (result == 0) {
            close(sock);
            free(recv_buffer);
            PyBuffer_Release(&atcmd);
            PyErr_SetString(PyExc_ValueError, "channel connection timeout.");
            return NULL;

        } else if (result < 0) {
            close(sock);
            free(recv_buffer);
            PyBuffer_Release(&atcmd);
            PyErr_SetString(PyExc_ValueError, "channel connection error.");
            return NULL;

        } else {

            // ready to write
            if (FD_ISSET(sock, &write_fds)) {
                sz2 = write(sock, (char *)atcmd.buf, (size_t)atcmd.len); 
                if (sz2 < atcmd.len) {
                    close(sock);
                    free(recv_buffer);
                    PyBuffer_Release(&atcmd);
                    PyErr_SetString(PyExc_ValueError, "it was not possible to send the data totally.");
                    return NULL;
                }
            }

            // ready to read
            if (FD_ISSET(sock, &read_fds)) {
                memset(recv_buffer, 0, sz);
                sz2 = recv(sock, recv_buffer, sz, 0);
                if (sz2 < 0) {
                    free(recv_buffer);
                    PyBuffer_Release(&atcmd);
                    PyErr_SetString(PyExc_ValueError, "no data received.");
                    return NULL;
                }

                close(sock);
                ret = PyBytes_FromStringAndSize(recv_buffer, sz2);
                free(recv_buffer);
                PyBuffer_Release(&atcmd);
                return ret;
            }

        } // socket ready
    } // select loop


}

static PyMethodDef ModMethods[] = {
    {"interfaces", interfaces, METH_VARARGS, "detect interfaces."},
    {"scan", scan, METH_VARARGS, "Scan for detecting devices."},
    //{"lescan", lescan, METH_VARARGS, "Scan for detecting LE devices."},
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

