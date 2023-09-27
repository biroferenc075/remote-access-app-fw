#include <iostream>
#include <string>

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>

#include "streamingClient.hpp"

using boost::asio::ip::tcp;
using namespace sc;
void run(string const& host, boost::asio::io_context& io_context) {
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints =
        resolver.resolve(host, "daytime");

    tcp::socket socket(io_context);
    
    boost::asio::connect(socket, endpoints);
    

    BFEWindow win{WIDTH, HEIGHT, "Vulkan" };
    BFEDevice dev{ win };

    StreamingClient streaming_client_(io_context, move(socket), win, dev);

    streaming_client_.start();
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            //std::cerr << "Usage: streamingclient <host>" << std::endl;
            //return 1;

            boost::asio::io_context io_context;
            run("localhost", io_context);
        }
        else {
            boost::asio::io_context io_context;
            run(string(argv[1]), io_context);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

