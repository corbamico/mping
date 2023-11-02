#include "ping.hpp"

#include <asio/experimental/awaitable_operators.hpp>

int main()
{
    using namespace asio::experimental::awaitable_operators;
    using namespace std::chrono_literals;

    asio::io_context io{};
    asio::signal_set signals(io, SIGINT, SIGTERM);


    //Ping pinger(io, "104.193.88.1/24");
    Ping pinger(io, "192.168.1.1/26");

    if (!pinger.valid_ip())
    {
        std::cerr << "usage: mping target"
                  << "help:  target as 127.0.0.1 or 192.168.0.1/24"
                  << std::endl;
    }
    signals.async_wait([&](auto, auto)
                       { pinger.stop(); });
    asio::co_spawn(io, pinger.ping_all() && pinger.recieve_all() , asio::detached);
    
    io.run();
    return 0;
}