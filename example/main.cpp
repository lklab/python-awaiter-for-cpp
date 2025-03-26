#include <iostream>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

#include "pyawaiter/manager.h"
#include "pyawaiter/async_invoker.h"

static boost::asio::io_context io_context;

boost::asio::awaitable<void> test(int id)
{
    pyawaiter::AsyncInvoker<std::string, int, const char*, double> invoker("example.example", "example_func");
    std::shared_ptr<std::string> result = co_await invoker.call(id, "example_string", 1.23);

    if (result)
    {
        std::cout << "[" << id << "] test result: " << *result.get() << std::endl;
    }
    else
    {
        std::cout << "[" << id << "] test result: nullptr" << std::endl;
    }
}

boost::asio::awaitable<void> waiter()
{
    boost::asio::steady_timer timer(io_context, std::chrono::seconds(20));
    co_await timer.async_wait(boost::asio::use_awaitable);
}

int main()
{
    pyawaiter::initialize(io_context);

    std::signal(SIGINT, [](int sig)
    {
        std::cout << "\nCaught SIGINT" << std::endl;
        io_context.stop();
    });

    for (int i = 0; i < 10; ++i)
    {
        boost::asio::co_spawn(
            io_context,
            test(i),
            boost::asio::detached
        );
    }

    boost::asio::co_spawn(
        io_context,
        waiter(),
        boost::asio::detached
    );

    io_context.run();

    pyawaiter::terminate();

    return 0;
}
