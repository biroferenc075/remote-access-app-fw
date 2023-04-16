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

sc::StreamingClient::StreamingClient(boost::asio::io_context& io_context, tcp::socket&& socket) : io_context_(io_context),
displayModule_(io_context, everyoneReady, readyCond, mut),
decompressionModule_(everyoneReady, readyCond, mut, displayModule_),
network_client_(socket, decompressionModule_, everyoneReady, readyCond, mut) {
}

void StreamingClient::start() {
    
    boost::thread t1(boost::bind(&tcp_client::readThread, &network_client_));

    boost::thread t4(boost::bind(&DecompressionModule::run, &decompressionModule_));

    boost::thread t3(boost::bind(&DisplayModule::run, &displayModule_));


    //boost::thread t2(boost::bind(&tcp_client::writeThread, &network_client_));

    t1.detach();
    //t2.detach();

    t3.detach();
    t4.detach();
    boost::unique_lock<boost::mutex> lock(mut);

    while (!(network_client_.ready() && decompressionModule_.ready() && displayModule_.ready()))
    {
        //readyCond.wait(lock);
        Sleep(100);
    }
    lock.unlock();
    everyoneReady = true;
    std::cout << "wait ready\n";
    readyCond.notify_all();
    std::cout << "io run\n";


    Sleep(1000);
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> guard = boost::asio::make_work_guard(io_context_);
    io_context_.run();

    std::cout << "exiting\n";
}

