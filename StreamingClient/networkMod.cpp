#include <iostream>
#include <string>

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>
#include "networkMod.hpp"

using boost::asio::ip::tcp;

using namespace sc;

const int imgsize = 600 * 800 * 4;

tcp_client::tcp_client(tcp::socket& socket, DecompressionModule& dm) : socket_(socket), dm(dm) {}
void tcp_client::readThread() {

    //boost::lock_guard<boost::mutex> lock(mut);
    std::cout << "netw read\n";
        boost::array<char, 1024> buf;

        size_t len = 0;
        size_t remainder = 0;

        while (!shouldStop)
        {
            size_t offs = 0;
            Frame* fr = new Frame(imgsize);
            memcpy(fr->data, buf.data() + len - remainder, remainder);
            
            while (offs < imgsize) {
                boost::system::error_code error;

                len = socket_.read_some(boost::asio::buffer(buf), error);

                if (error == boost::asio::error::eof) {
                    shouldStop = true;
                    break; // Connection closed cleanly by peer.
                }
                else if (error) {
                    shouldStop = true;
                    throw boost::system::system_error(error); // Some other error.
                }

                remainder = offs + len - imgsize;
                if (remainder > 0) {
                    memcpy(fr->data + offs, buf.data(), len-remainder);
                }
                else {
                    memcpy(fr->data + offs, buf.data(), len);
                }

                offs += len;
            }

            dm.submitToQueue(fr);
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




