#ifndef PYAWAITER_UTILS_H
#define PYAWAITER_UTILS_H

#include <functional>
#include <boost/asio.hpp>
#include <Python.h>

namespace pyawaiter
{
    void set_io_context(boost::asio::io_context& io_context);
    boost::asio::io_context& get_io_context();

    int new_request(std::shared_ptr<boost::asio::steady_timer>& timer);
    template<typename T>
    std::shared_ptr<T> get_result(int rqid);
    void clear_request(int rqid);

    void on_cpp_callback(PyObject* p_args);

    void call_python_function(PyObject* p_func, PyObject* p_args);
    void call_python_function(PyObject* p_func);

    template<typename T>
    PyObject* to_py_object(const T& val);
    template<>
    PyObject* to_py_object(const int& val);
    template<>
    PyObject* to_py_object(const double& val);
    template<>
    PyObject* to_py_object(const char* const& val);
    template<>
    PyObject* to_py_object(const std::string& val);

    template<typename T>
    std::shared_ptr<T> parse_py_object(PyObject* p_args);
    template<>
    std::shared_ptr<int> parse_py_object(PyObject* p_args);
    template<>
    std::shared_ptr<std::string> parse_py_object(PyObject* p_args);

    void ensure_gil();
    void release_gil();

    PyObject* get_args(int rqid);
}

namespace pyawaiter
{
    template<typename T>
    std::shared_ptr<T> get_result(int rqid)
    {
        PyObject* p_args = get_args(rqid);
        if (p_args == nullptr) return nullptr;

        return parse_py_object<T>(p_args);
    }
}

#endif
