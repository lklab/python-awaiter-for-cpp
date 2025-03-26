#ifndef PYAWAITER_ASYNC_INVOKER_H
#define PYAWAITER_ASYNC_INVOKER_H

#include <boost/asio.hpp>
#include <Python.h>

#include "pyawaiter/utils.h"

namespace pyawaiter
{
    template<typename TResult, typename... Args>
    class AsyncInvoker
    {
    public:
        AsyncInvoker(PyObject* p_method) : p_method(p_method) { }

        AsyncInvoker(const char* module, const char* func)
        {
            ensure_gil();
            PyObject* p_module = PyImport_ImportModule(module);
            p_method = PyObject_GetAttrString(p_module, func);
            Py_XDECREF(p_module);
            release_gil();
        }

        virtual ~AsyncInvoker()
        {
            ensure_gil();
            Py_XDECREF(p_method);
            release_gil();
        }

        boost::asio::awaitable<std::shared_ptr<TResult>> call(const Args&... args, int timeout = 10)
        {
            std::shared_ptr<boost::asio::steady_timer> timer = std::make_shared<boost::asio::steady_timer>(get_io_context(), std::chrono::seconds(timeout));
            int rqid = new_request(timer);

            ensure_gil();
            constexpr size_t N = sizeof...(Args);
            PyObject* py_objects[N + 1];
            py_objects[0] = PyLong_FromLong(rqid);
            size_t i = 1;
            ((py_objects[i++] = to_py_object(args)), ...);
            PyObject* p_args = PyTuple_New(static_cast<int>(N + 1));
            for (size_t j = 0; j < N + 1; ++j)
            {
                PyTuple_SET_ITEM(p_args, j, py_objects[j]);
            }

            call_python_function(p_method, p_args);

            Py_DECREF(p_args);
            release_gil();

            try
            {
                co_await timer->async_wait(boost::asio::use_awaitable);
            }
            catch (std::exception& e) { }
            timer.reset();

            std::shared_ptr<TResult> result = get_result<TResult>(rqid);
            clear_request(rqid);
            co_return result;
        }

    private:
        PyObject* p_method;
    };
}

#endif
