#include <iostream>
#include <string>

#include "networkMod.hpp"
#include "streamingClient.hpp"

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>
using namespace std;
using namespace sc;

using boost::asio::ip::tcp;

sc::StreamingClient::StreamingClient(boost::asio::io_context& io_context, tcp::socket socket) : io_context_(io_context),
displayModule_(io_context, dispThreadReady, readyCond, mut),
decompressionModule_(decompThreadReady, readyCond, mut, displayModule_),
network_client_(socket, decompressionModule_) {
}

void StreamingClient::start() {

    boost::thread t3(boost::bind(&DisplayModule::run, &displayModule_));

    boost::thread t4(boost::bind(&DecompressionModule::run, &decompressionModule_));

    boost::thread t1(boost::bind(&tcp_client::readThread, &network_client_));

    //boost::thread t2(boost::bind(&tcp_client::writeThread, &network_client_));

    t1.detach();

    //t2.detach();

    t3.detach();

    t4.detach();
    boost::unique_lock<boost::mutex> lock(mut);

    decompThreadReady = writeThreadReady = readThreadReady = true;
    while (!(writeThreadReady && readThreadReady && dispThreadReady && decompThreadReady))
    {
        readyCond.wait(lock);
    }
    std::cout << "wait ready\n";
    readyCond.notify_all();

    std::cout << "io run\n";
    io_context_.run();
}

