#include <iostream>
#include <string>

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>
#include "networkMod.hpp"

using boost::asio::ip::tcp;

using namespace sc;

tcp_client::tcp_client(tcp::socket& socket) : socket_(socket) {}
void tcp_client::readThread() {
        while (!shouldStop)
        {
            boost::array<char, 1024> buf;
            boost::system::error_code error;

            size_t len = socket_.read_some(boost::asio::buffer(buf), error);

            if (error == boost::asio::error::eof) {
                shouldStop = true;
                break; // Connection closed cleanly by peer.
            }
            else if (error) {
                shouldStop = true;
                throw boost::system::system_error(error); // Some other error.
            }
                
            std::cout.write(buf.data(), len);
            std::cout << std::endl;
        }
    }

    void tcp_client::writeThread() {
        std::string inp;
        do {
            std::cin >> inp;

            boost::asio::async_write(socket_, boost::asio::buffer(inp), 
                boost::bind(&tcp_client::handle_write, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
        } while (inp.at(0) != 'x' && inp.length() == 1 && !shouldStop);
        shouldStop = true;
    }

    void tcp_client::handle_write(const boost::system::error_code& /*error*/,
        size_t /*bytes_transferred*/)
    {
    }




