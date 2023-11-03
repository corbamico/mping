#if defined(_WIN32) || defined(__CYGWIN__)
#include <sdkddkver.h>
#endif
#ifdef __clang__
#define ASIO_HAS_CO_AWAIT
#endif
#include <asio.hpp>
#include <iostream>
#include <chrono>
#include <cmath>
#include <regex>

#include "icmp_header.hpp"
#include "ipv4_header.hpp"

using namespace std::chrono_literals;
constexpr auto use_nothrow_awaitable = asio::as_tuple(asio::use_awaitable);
using async_void = asio::awaitable<void>;
using asio::ip::icmp;
using std::chrono::steady_clock;

class Ping:public std::enable_shared_from_this<Ping>
{

private:
    asio::io_context &io_;
    asio::ip::icmp::resolver resolver_;
    asio::ip::icmp::socket socket_;
    asio::steady_timer timer_;
    u_short identifier_;
    u_short sequence_number_;
    u_short recieved_;

    u_short sent_;
    u_short reached_;
    asio::streambuf reply_buffer_;
    std::vector<icmp::endpoint> addresses_;
    std::unordered_map<icmp::endpoint,steady_clock::time_point> addresses_map_;


public:
    Ping(asio::io_context &io, std::string_view sv)
        : io_{io}, resolver_{io}, socket_{io, asio::ip::icmp::v4()},
          timer_{io},
          identifier_{0}, sequence_number_{0}, recieved_{0},
          sent_{0},reached_{0}
    {
        generate_ips(sv);
    }

    async_void ping_all()
    {
        std::cout << "Ping address range from: " << addresses_.begin()->address().to_string()
                  << " to: " << addresses_.rbegin()->address().to_string()
                  << std::endl;

        for (auto address : addresses_)
        {
            co_await ping_once(address);
            timer_.expires_after(1ms);
            co_await timer_.async_wait(use_nothrow_awaitable);
        }
        timer_.expires_after(3000ms);
        timer_.async_wait([this](const asio::error_code&){this->stop();});
        co_return;
    }

    async_void recieve_all()
    {
        while (recieved_ < addresses_.size())
        {
            co_await recieve_once();
        }
        co_return;
    }
    void stop()
    {
        std::cout << "\nPing statistics: " << std::endl
                  << "    Packet: sent = " << sent_
                  << ", reached = " << reached_
                  << std::endl;
        io_.stop();
    }

private:
    async_void ping_once(asio::ip::icmp::endpoint address)
    {
        
        //std::unordered_map<asio::ip::icmp::endpoint,std::time_t>;
        // Create an ICMP header for an echo request.
        std::string body("ping from asio.");
        icmp_header echo_request;
        echo_request.type(icmp_header::echo_request);
        echo_request.code(0);
        echo_request.identifier(identifier_++);
        echo_request.sequence_number(sequence_number_++);
        compute_checksum(echo_request, body.begin(), body.end());

        // Encode the request packet.
        asio::streambuf request_buffer;
        std::ostream os(&request_buffer);
        os << echo_request << body;
        sent_++;
        
        addresses_map_[address] = steady_clock::now();
        //auto [ec1, transferred] = co_await socket_.async_send_to(request_buffer.data(), address, use_nothrow_awaitable);
        co_await socket_.async_send_to(request_buffer.data(), address, use_nothrow_awaitable);
        
    }

    async_void recieve_once()
    {
        if (!socket_.is_open())
        {
            socket_.open(asio::ip::icmp::v4());
        }

        asio::ip::icmp::endpoint remote_endpoint;
        reply_buffer_.consume(reply_buffer_.size());

        auto [ec, bytes_transferred] = co_await socket_.async_receive_from(reply_buffer_.prepare(65536), remote_endpoint, use_nothrow_awaitable);

        if (!ec && bytes_transferred > 0)
        {
            ipv4_header ipv4_hdr;
            icmp_header icmp_hdr;
            reply_buffer_.commit(bytes_transferred);
            std::istream is(&reply_buffer_);
            is >> ipv4_hdr >> icmp_hdr;

            if ((ipv4_hdr.protocol() == 1) && (ipv4_hdr.type_of_service() == 0))
            {

                if (icmp_hdr.type() == icmp_header::echo_reply)
                {
                    reached_++;
                    recieved_++;
                    steady_clock::time_point start_time{steady_clock::now()};
                    icmp::endpoint endpoint(ipv4_hdr.source_address(),0);
                    auto it = addresses_map_.find(endpoint);
                    if (it != addresses_map_.end())
                    {
                        start_time = it->second;
                    }

                    std::cout << "Reply from " << ipv4_hdr.source_address().to_string()
                              << ": bytes=" << bytes_transferred
                              << " time=" 
                              // std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time_).count()
                              << std::chrono::duration_cast<std::chrono::milliseconds>(steady_clock::now() - start_time).count()
                              << "ms"
                              << " TTL=" << ipv4_hdr.time_to_live()
                              << std::endl;
                }
                else if ((icmp_hdr.type() == icmp_header::destination_unreachable) || (icmp_hdr.type() == icmp_header::time_exceeded))
                {
                    // https://www.rfc-editor.org/rfc/rfc792
                    //  if type = Destination Unreachable Message , then data is
                    //      Internet Header + 64 bits of Original Data Datagram
                    recieved_++;
                    is >> ipv4_hdr >> icmp_hdr;
                    if ((ipv4_hdr.version() == 4) && (ipv4_hdr.protocol() == 1))
                    {
                        // std::cout << "Destination unreachable " << ipv4_hdr.destination_address().to_string()
                        //           << std::endl;
                    }
                }
            }
        }
    }

    /// @brief generate IPs according to '192.168.1.*'
    /// @param sv
    void generate_ips(std::string_view sv)
    {
        std::string ip_base;
        int subnet = 32;
        if (sv.find('*') != std::string::npos)
        {
            ip_base = std::string(sv).substr(0, sv.find_last_of('.'));

            int start = 1;
            int end = 254;

            for (int i = start; i <= end; i++)
            {
                std::string ip = ip_base + "." + std::to_string(i);
                addresses_.push_back(*resolver_.resolve(asio::ip::icmp::v4(), ip, "").begin());
            }
        }
        else
        {
            addresses_.push_back(*resolver_.resolve(asio::ip::icmp::v4(), sv, "").begin());
        }
        //prevent too much address.
        if (addresses_.size()>254)
            addresses_.clear();
    }
public:
    bool valid_ip()
    {
        return addresses_.size()>0;
    }
};

bool isValidIP(const std::string& ip) {
    std::regex IPv4Pattern1(
        "^([0-9]{1,2}|1[0-9]{2}|2[0-4][0-9]|25[0-5])"
        "\\."
        "([0-9]{1,2}|1[0-9]{2}|2[0-4][0-9]|25[0-5])"                   
        "\\."
        "([0-9]{1,2}|1[0-9]{2}|2[0-4][0-9]|25[0-5])"                    
        "\\."
        "\\*$" 
    );
    std::regex IPv4Pattern2(
        "^([0-9]{1,2}|1[0-9]{2}|2[0-4][0-9]|25[0-5])"
        "\\."
        "([0-9]{1,2}|1[0-9]{2}|2[0-4][0-9]|25[0-5])"                   
        "\\."
        "([0-9]{1,2}|1[0-9]{2}|2[0-4][0-9]|25[0-5])"                    
        "\\."
        "([0-9]{1,2}|1[0-9]{2}|2[0-4][0-9])$" 
    );
    return std::regex_match(ip, IPv4Pattern1) || std::regex_match(ip, IPv4Pattern2);
}
