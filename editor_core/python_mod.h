#ifndef PYTHON_MOD_H
#define PYTHON_MOD_H

#include <Python.h>
#include <structmember.h>
#include <stdlib.h>
#include "buffer.h"
#include "ui.h"
#include "logging.h"

extern struct UI_Pane *active_pane;

typedef struct {
    PyObject_HEAD
    struct Buffer **buffer;
    PyObject *cursor;
    PyObject *keymap;
    PyObject *dict;
} PyBuffer;

static PyMemberDef PyBuffer_members[] = {
    {NULL}  /* Sentinel */
};

static PyObject *PyBuffer_getcursor(PyBuffer *self, void *closure)
{
    struct Buffer *buf = *(self->buffer);
    PyObject *cursor = Py_BuildValue("(ii)", buf->cursor_x, buf->cursor_y);

    return cursor;
}

static int PyBuffer_setcursor(PyBuffer *self, PyObject *value, void *closure)
{
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete the cursor attribute");
        return -1;
    }

    int x, y;
    if (!PyArg_ParseTuple(value, "ii:setcursor", &x, &y)) {
        PyErr_SetString(PyExc_TypeError,
                        "Cursor must be a tuple of 2 elements.");
        return -1;
    }

    struct Buffer *buf = *(self->buffer);
    buffer_move_cursor_y(buf, y - buf->cursor_y);
    buffer_move_cursor_x(buf, x - buf->cursor_x);

    return 0;
}

static PyObject *PyBuffer_getkeymap(PyBuffer *self, void *closure)
{
    Py_INCREF(active_pane->keymap);
    return active_pane->keymap;
}

static int PyBuffer_setkeymap(PyBuffer *self, PyObject *value, void *closure)
{
    if (active_pane->keymap != NULL)
        Py_DECREF(active_pane->keymap);

    Py_INCREF(value);
    active_pane->keymap = value;

    return 0;
}

static PyGetSetDef PyBuffer_getseters[] = {
    {"cursor",
     (getter)PyBuffer_getcursor, (setter)PyBuffer_setcursor,
     "the cursor", NULL},
    {"keymap",
     (getter)PyBuffer_getkeymap, (setter)PyBuffer_setkeymap},
    {"__dict__", PyObject_GenericGetDict, PyObject_GenericSetDict},
    {NULL}  /* Sentinel */
};

static PyObject *PyBuffer_insert(PyBuffer *self, PyObject *args)
{
    char *str;
    int n;

    if (!PyArg_ParseTuple(args, "s:insert", &str)) {
        return NULL;
    }

    n = strlen(str);

    for (int i = 0; i < n; ++i) {
        if (str[i] == '\n')
            buffer_break_at_cursor(*(self->buffer));
        else
            buffer_insert(*(self->buffer), str[i]);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *PyBuffer_delete_backwards(PyBuffer *self, PyObject *args)
{
    int n;

    if (!PyArg_ParseTuple(args, "i:insert", &n)) {
        return NULL;
    }

    buffer_delete_backwards(*(self->buffer), n);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *PyBuffer_get_line(PyBuffer *self, PyObject *args)
{
    int y;

    if (!PyArg_ParseTuple(args, "i:insert", &y)) {
        return NULL;
    }

    char *display = line_display(buffer_nth_line(*(self->buffer), y));
    return Py_BuildValue("s", display);
}

static PyMethodDef PyBuffer_methods[] = {
    {"insert", (PyCFunction)PyBuffer_insert, METH_VARARGS,
     "Insert the given string to the buffer at the cursor location."
    },
    {"delete_backwards", (PyCFunction)PyBuffer_delete_backwards, METH_VARARGS,
     "Delete n characters backwards from the cursor."
    },
    {"get_line", (PyCFunction)PyBuffer_get_line, METH_VARARGS,
     "Get the contents of a given line."
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject PyBufferType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "editor.Buffer",             /* tp_name */
    sizeof(PyBuffer),            /* tp_basicsize */
    0,                           /* tp_itemsize */
    0,                           /* tp_dealloc */
    0,                           /* tp_print */
    0,                           /* tp_getattr */
    0,                           /* tp_setattr */
    0,                           /* tp_reserved */
    0,                           /* tp_repr */
    0,                           /* tp_as_number */
    0,                           /* tp_as_sequence */
    0,                           /* tp_as_mapping */
    0,                           /* tp_hash  */
    0,                           /* tp_call */
    0,                           /* tp_str */
    PyObject_GenericGetAttr,     /* tp_getattro */
    PyObject_GenericSetAttr,     /* tp_setattro */
    0,                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,          /* tp_flags */
    "A buffer representation",   /* tp_doc */
    0,                           /* tp_traverse */
    0,                           /* tp_clear */
    0,                           /* tp_richcompare */
    0,                           /* tp_weaklistoffset */
    0,                           /* tp_iter */
    0,                           /* tp_iternext */
    PyBuffer_methods,            /* tp_methods */
    PyBuffer_members,            /* tp_members */
    PyBuffer_getseters,          /* tp_getset */
    0,                           /* tp_base */
    0,                           /* tp_dict */
    0,                           /* tp_descr_get */
    0,                           /* tp_descr_set */
    offsetof(PyBuffer, dict),    /* tp_dictoffset */
    0,                           /* tp_init */
    0,                           /* tp_alloc */
    PyType_GenericNew,           /* tp_new */
};

static PyMethodDef editorMethods[] = {
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static PyModuleDef editormodule = {
    PyModuleDef_HEAD_INIT,
    "editor",
    "Module for manipulating the editor.",
    -1,
    editorMethods,
    NULL, NULL, NULL, NULL
};

#endif /* PYTHON_MOD_H */
