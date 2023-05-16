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

sc::StreamingClient::StreamingClient(boost::asio::io_context& io_context, tcp::socket&& socket, BFEWindow& bfeWindow, BFEDevice& bfeDevice) : 
io_context_(io_context), bfeWindow(bfeWindow), bfeDevice(bfeDevice),
network_client_(socket, decompressionModule_, everyoneReady, readyCond, mut), 
displayModule_(io_context, everyoneReady, readyCond, mut, bfeWindow, bfeDevice),
decompressionModule_(everyoneReady, readyCond, mut, displayModule_, bfeDevice) {
}

void StreamingClient::start() {
    try {
    boost::thread t1(boost::bind(&tcp_client::readThread, &network_client_));

    boost::thread t2(boost::bind(&DecompressionModule::run, &decompressionModule_));

    t1.detach();

    t2.detach();
    boost::thread r(boost::bind(&StreamingClient::waitUntilReady, this));
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> guard = boost::asio::make_work_guard(io_context_);

    boost::thread io(boost::bind(&boost::asio::io_context::run, &io_context_));
    io.detach();

    displayModule_.run();
    }
    catch (std::exception e) {
        std::cout << e.what();
    }
}

void StreamingClient::waitUntilReady() {
    boost::unique_lock<boost::mutex> lock(mut);

    while (!(network_client_.ready() && decompressionModule_.ready() && displayModule_.ready()))
    {
        Sleep(100);
    }
    lock.unlock();
    everyoneReady = true;
    readyCond.notify_all();
    Sleep(1000);

}
