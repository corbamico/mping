#include "ping.hpp"

//#include <asio/experimental/awaitable_operators.hpp>

int main()
{
    //using namespace asio::experimental::awaitable_operators;
    using namespace std::chrono_literals;

    asio::io_context io{};
    asio::signal_set signals(io, SIGINT, SIGTERM);


    //Ping pinger(io, "104.193.88.1/24");
    Ping pinger(io, "192.168.2.1/24");

    signals.async_wait([&](auto, auto)
                       { pinger.stop(); });
    asio::steady_timer timer(io, 20s);
    timer.async_wait([&](auto ec)
                     { pinger.stop(); });

    asio::co_spawn(io, pinger.ping_all() , asio::detached);
    asio::co_spawn(io, pinger.recieve_all() , asio::detached);    

    io.run();
    return 0;
}