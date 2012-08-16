#include <Python.h>
#include "dablooms.h"
#include "structmember.h"

int Py_ModuleVersion = 1;

typedef struct {
    PyObject_HEAD
    scaling_bloom_t *filter;    /* Type-specific fields go here. */
} Dablooms;

static void Dablooms_dealloc(Dablooms *self)
{
    free_scaling_bloom(self->filter);
    self->ob_type->tp_free((PyObject *)self);
}

static PyObject *Dablooms_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Dablooms *self;
    
    if ((self = (Dablooms *)type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }
    
    self->filter = NULL;
    
    return (PyObject *) self;
}

static int Dablooms_init(Dablooms *self, PyObject *args, PyObject *kwds)
{
    double error_rate;
    const char *filepath;
    unsigned int capacity;
    int id;
    static char *kwlist[] = {"capacity", "error_rate", "filepath", NULL};
    
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|ids", kwlist,
                                      &capacity, &error_rate, &filepath)) {
        return -1;
    }
    
    self->filter = new_scaling_bloom(capacity, error_rate, filepath);
    
    return 0;
}

static PyObject *check(Dablooms *self, PyObject *args)
{
    const char *hash;
    int len;
    
    if (!PyArg_ParseTuple(args, "s#", &hash, &len)) {
        return NULL;
    }
    return Py_BuildValue("i", scaling_bloom_check(self->filter, hash, len));
}

static PyObject *add(Dablooms *self, PyObject *args, PyObject *kwds)
{
    const char *hash;
    int id, len;
    
    static char *kwlist[] = {"hash", "id", NULL};
    
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|s#i", kwlist, &hash, &len, &id)) {
        return NULL;
    }
    
    return Py_BuildValue("i", scaling_bloom_add(self->filter, hash, len, id));
}

static PyObject *delete(Dablooms *self, PyObject *args, PyObject *kwds)
{
    const char *hash;
    int id, len;
    static char *kwlist[] = {"hash", "id", NULL};
    
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|s#i", kwlist, &hash, &len, &id)) {
        return NULL;
    }
    
    return Py_BuildValue("i", scaling_bloom_remove(self->filter, hash, len, id));
}

static PyObject *flush(Dablooms *self, PyObject *args, PyObject *kwds)
{
    return Py_BuildValue("i", scaling_bloom_flush(self->filter));
}

static PyObject *pre_seqnum(Dablooms *self, PyObject *args, PyObject *kwds)
{
    return Py_BuildValue("l", *self->filter->header->preseq);
}

static PyObject *post_seqnum(Dablooms *self, PyObject *args, PyObject *kwds)
{
    return Py_BuildValue("l", *self->filter->header->posseq);
}

static PyMethodDef Dablooms_methods[] = {
    {"add",         (PyCFunction)add,         METH_VARARGS | METH_KEYWORDS, "Add an element to the bloom filter."},
    {"delete",      (PyCFunction)delete,      METH_VARARGS | METH_KEYWORDS, "Remove an element from the bloom filter."},
    {"check",       (PyCFunction)check,       METH_VARARGS | METH_KEYWORDS, "Check if an element is in the bloom filter."},
    {"flush",       (PyCFunction)flush,       METH_VARARGS | METH_KEYWORDS, "Flush a bloom filter to file."},
    {"pre_seqnum",  (PyCFunction)pre_seqnum,  METH_VARARGS | METH_KEYWORDS, "Get the pre-sequence number."},
    {"post_seqnum", (PyCFunction)post_seqnum, METH_VARARGS | METH_KEYWORDS, "Get the post-sequence number."},
    {NULL},       /* Sentinel */
};

static PyMemberDef Dablooms_members[] = {
    {NULL}  /* Sentinel */
};

static PyTypeObject DabloomsType = {
    PyObject_HEAD_INIT(NULL)
    0,                              /*ob_size*/
    "pydablooms.Dablooms",          /*tp_name*/
    sizeof(Dablooms),               /*tp_basicsize*/
    0,                              /*tp_itemsize*/
    (destructor)Dablooms_dealloc,   /*tp_dealloc*/
    0,                              /*tp_print*/
    0,                              /*tp_getattr*/
    0,                              /*tp_setattr*/
    0,                              /*tp_compare*/
    0,                              /*tp_repr*/
    0,                              /*tp_as_number*/
    0,                              /*tp_as_sequence*/
    0,                              /*tp_as_mapping*/
    0,                              /*tp_hash*/
    0,                              /*tp_call*/
    0,                              /*tp_str*/
    0,                              /*tp_getattro*/
    0,                              /*tp_setattro*/
    0,                              /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,             /*tp_flags*/
    "Dablooms objects",             /*tp_doc*/
    0,                              /*tp_traverse*/
    0,                              /*tp_clear*/
    0,                              /*tp_richcompare*/
    0,                              /*tp_weaklistoffset*/
    0,                              /*tp_iter*/
    0,                              /*tp_iternext*/
    Dablooms_methods,               /*tp_methods*/
    Dablooms_members,               /*tp_members*/
    0,                              /*tp_getset*/
    0,                              /*tp_base*/
    0,                              /*tp_dict*/
    0,                              /*tp_descr_get*/
    0,                              /*tp_descr_set*/
    0,                              /*tp_dictoffset*/
    (initproc)Dablooms_init,        /*tp_init*/
    0,                              /*tp_alloc*/
    Dablooms_new,                   /*tp_new*/
};

static PyObject *load_dabloom(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Dablooms *self = (Dablooms *)PyObject_New(Dablooms, &DabloomsType);
    double error_rate;
    const char *filepath;
    unsigned int capacity;
    static char *kwlist[] = {"capacity", "error_rate", "filepath", NULL};
    
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|ids", kwlist,
                                      &capacity, &error_rate, &filepath)) {
        return NULL;
    }
    
    self->filter = new_scaling_bloom_from_file(capacity, error_rate, filepath);
    return (PyObject *) self;
}

static PyMethodDef pydablooms_methods[] = {
    {"load_dabloom", (PyCFunction)load_dabloom, METH_VARARGS | METH_KEYWORDS, "Add an element to the bloom filter."},
    {NULL}
};

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif


PyMODINIT_FUNC initpydablooms(void)
{
    PyObject *m;
    if (PyType_Ready(&DabloomsType) < 0) {
        return;
    }
    
    m = Py_InitModule3("pydablooms", pydablooms_methods, "Dablooms module");
    
    if (m == NULL) {
        return;
    }
    
    PyModule_AddObject(m, "__version__", PyString_FromString(dablooms_version()));
    
    Py_INCREF(&DabloomsType);
    PyModule_AddObject(m, "Dablooms", (PyObject *)&DabloomsType);
}
