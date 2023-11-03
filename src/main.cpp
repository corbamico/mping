#include "ping.hpp"

#include <asio/experimental/awaitable_operators.hpp>

void usage()
{
    std::cerr << "usage: mping target\n"
              << "help:  target as 192.168.0.1 or 192.168.0.*"
              << std::endl;
    exit(1);
}

int main(int argc, char *argv[])
{
    using namespace asio::experimental::awaitable_operators;
    if ((argc!=2)||(!isValidIP(argv[1])))
    {
        usage();
    }

    asio::io_context io{};
    asio::signal_set signals(io, SIGINT, SIGTERM);

    Ping pinger(io, argv[1]);
    if (!pinger.valid_ip())
    {
        usage();
    }
    
    signals.async_wait([&](auto, auto)
                       { pinger.stop(); });
    asio::co_spawn(io, pinger.ping_all() && pinger.recieve_all(), asio::detached);

    io.run();
    return 0;
}