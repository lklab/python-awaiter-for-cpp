#include "pyawaiter/utils.h"

#include <unordered_map>

static boost::asio::io_context* g_io_context;

static PyGILState_STATE g_gil_state;
static int g_gil_count = 0;

static int g_rqid_counter = 0;
static std::unordered_map<int, std::weak_ptr<boost::asio::steady_timer>> g_timers;
static std::unordered_map<int, PyObject*> g_results;

namespace pyawaiter
{
    void set_io_context(boost::asio::io_context& io_context)
    {
        g_io_context = &io_context;
    }

    boost::asio::io_context& get_io_context()
    {
        return *g_io_context;
    }

    int new_request(std::shared_ptr<boost::asio::steady_timer>& timer)
    {
        int rqid = g_rqid_counter;
        ++g_rqid_counter;
        if (g_rqid_counter < 0) g_rqid_counter = 0;

        g_timers[rqid] = timer;

        return rqid;
    }

    void clear_request(int rqid)
    {
        auto timer_it = g_timers.find(rqid);
        if (timer_it != g_timers.end())
        {
            g_timers.erase(rqid);

            if (auto timer = timer_it->second.lock())
            {
                timer->cancel();
                timer.reset();
            }
        }

        auto result_it = g_results.find(rqid);
        if (result_it != g_results.end())
        {
            g_results.erase(rqid);

            ensure_gil();
            Py_XDECREF(result_it->second);
            release_gil();
        }
    }

    void on_cpp_callback(PyObject* p_args)
    {
        int rqid;
        PyObject* p_arg;

        if (!PyArg_ParseTuple(p_args, "iO", &rqid, &p_arg))
        {
            return;
        }

        Py_INCREF(p_args);

        boost::asio::post(get_io_context(), [rqid, p_args]()
        {
            auto timer_it = g_timers.find(rqid);
            if (timer_it != g_timers.end())
            {
                if (auto timer = timer_it->second.lock())
                {
                    g_results[rqid] = p_args;
                    timer->cancel();
                    timer.reset();
                    return; // success
                }
            }

            // fail
            ensure_gil();
            Py_XDECREF(p_args);
            release_gil();
        });
    }

    void call_python_function(PyObject* p_func, PyObject* p_args)
    {
        if (!p_func || !PyCallable_Check(p_func))
        {
            return;
        }

        PyObject* p_value = PyObject_CallObject(p_func, p_args);

        if (p_value == nullptr)
        {
            PyErr_Print();
        }
        else
        {
            Py_DECREF(p_value);
        }
    }

    void call_python_function(PyObject* p_func)
    {
        PyObject* p_args = PyTuple_New(0);
        call_python_function(p_func, p_args);
        Py_DECREF(p_args);
    }

    template<>
    PyObject* to_py_object(const int& val)
    {
        return PyLong_FromLong(val);
    }
    
    template<>
    PyObject* to_py_object(const double& val)
    {
        return PyFloat_FromDouble(val);
    }
    
    template<>
    PyObject* to_py_object(const char* const& val)
    {
        return PyUnicode_FromString(val);
    }
    
    template<>
    PyObject* to_py_object(const std::string& val)
    {
        return PyUnicode_FromString(val.c_str());
    }

    template<>
    std::shared_ptr<int> parse_py_object(PyObject* p_args)
    {
        int rqid;
        int result;

        if (!PyArg_ParseTuple(p_args, "ii", &rqid, &result))
        {
            return nullptr;
        }

        return std::make_shared<int>(result);
    }

    template<>
    std::shared_ptr<std::string> parse_py_object(PyObject* p_args)
    {
        int rqid;
        const char* c_str;

        if (!PyArg_ParseTuple(p_args, "is", &rqid, &c_str))
        {
            return nullptr;
        }

        return std::make_shared<std::string>(c_str);
    }

    void ensure_gil()
    {
        if (g_gil_count == 0)
        {
            g_gil_state = PyGILState_Ensure();
        }

        ++g_gil_count;
    }

    void release_gil()
    {
        if (g_gil_count == 0) return;

        --g_gil_count;

        if (g_gil_count == 0)
        {
            PyGILState_Release(g_gil_state);
        }
    }

    PyObject* get_args(int rqid)
    {
        auto it = g_results.find(rqid);
        if (it != g_results.end())
        {
            return it->second;
        }

        return nullptr;
    }
}
