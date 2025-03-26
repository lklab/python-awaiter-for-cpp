#include "pyawaiter/manager.h"

#include <Python.h>

#include "pyawaiter/utils.h"

static bool g_initialized = false;
static PyObject* p_module;

static PyObject* cpp_callback(PyObject* self, PyObject* p_args)
{
    pyawaiter::on_cpp_callback(p_args);
    Py_RETURN_NONE;
}

static PyMethodDef cppmethod[] = {
    {"cpp_callback", cpp_callback, METH_VARARGS, "cpp_callback"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef cppmodule = {
    PyModuleDef_HEAD_INIT,
    "cppmodule",
    NULL,
    -1,
    cppmethod
};

PyMODINIT_FUNC PyInit_cppmodule(void)
{
    return PyModule_Create(&cppmodule);
}

namespace pyawaiter
{
    void initialize(boost::asio::io_context& io_context)
    {
        if (g_initialized)
        {
            return;
        }

        set_io_context(io_context);

        PyImport_AppendInittab("cppmodule", PyInit_cppmodule);
        Py_Initialize();

        PyRun_SimpleString("import sys");
        PyRun_SimpleString("import os");
        PyRun_SimpleString("sys.path.append(os.getcwd())");

        p_module = PyImport_ImportModule("python.manager");
        if (!p_module)
        {
            PyErr_Print();
            return;
        }

        PyObject* p_func_initialize = PyObject_GetAttrString(p_module, "initialize");
        call_python_function(p_func_initialize);

        Py_XDECREF(p_func_initialize);

        PyEval_SaveThread();

        g_initialized = true;
    }

    void terminate()
    {
        if (!g_initialized)
        {
            return;
        }

        ensure_gil();

        PyErr_CheckSignals();
        PyErr_Clear();

        PyObject* p_func_terminate = PyObject_GetAttrString(p_module, "terminate");
        call_python_function(p_func_terminate);
        Py_XDECREF(p_func_terminate);

        Py_XDECREF(p_module);

        release_gil();

        ensure_gil();
        Py_Finalize();

        g_initialized = false;
    }
}
