#ifndef PYAWAITER_MANAGER_H
#define PYAWAITER_MANAGER_H

#include <boost/asio.hpp>

namespace pyawaiter
{
    void initialize(boost::asio::io_context& io_context);
    void terminate();
}

#endif
