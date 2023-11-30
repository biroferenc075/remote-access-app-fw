#include <iostream>
#include <string>

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>

#include "RemoteClient.hpp"
#include "inputevents.hpp"

using boost::asio::ip::tcp;
using namespace rc;
void run(string const& host, boost::asio::io_context& io_context) {
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints =
        resolver.resolve(host, "daytime");

    tcp::socket socket(io_context);
    
    boost::asio::connect(socket, endpoints);
    

    RACWindow win{WIDTH, HEIGHT, "Vulkan" };
    RACDevice dev{ win };

    RemoteClient remote_client(io_context, move(socket), win, dev);

    remote_client.registerCallbacks(win);

    remote_client.start();
}

int main(int argc, char* argv[])
{
    int tries = 0;
    while (tries++ < 5) {
        try
        {
            if (argc != 2)
            {
                //std::cerr << "Usage: streamingclient <host>" << std::endl;
                //return 1;

                boost::asio::io_context io_context;
                run("localhost", io_context);
                break;
            }
            else {
                boost::asio::io_context io_context;
                run(string(argv[1]), io_context);
                break;
            }
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            Sleep(2000);
        }
    }
}

