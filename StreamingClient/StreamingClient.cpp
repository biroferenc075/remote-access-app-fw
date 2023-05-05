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
    std::cout << "sc ctor\n";
}

void StreamingClient::start() {
    try {
    std::cout << "start!\n";
    boost::thread t1(boost::bind(&tcp_client::readThread, &network_client_));

    boost::thread t4(boost::bind(&DecompressionModule::run, &decompressionModule_));

    //boost::thread t3(boost::bind(&DisplayModule::run, &displayModule_));
    std::cout << t1.get_id() << " netw id\n";
    std::cout << t4.get_id() << " decomp id\n";
    
    //boost::thread t2(boost::bind(&tcp_client::writeThread, &network_client_));

    t1.detach();
    //t2.detach();

    //t3.detach();
    t4.detach();
    boost::thread r(boost::bind(&StreamingClient::waitUntilReady, this));
    std::cout << r.get_id() << " io id\n";
    std::cout << "io context run \n";
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> guard = boost::asio::make_work_guard(io_context_);

    //io_context.run();
    boost::thread io(boost::bind(&boost::asio::io_context::run, &io_context_));
    io.detach();

    std::cout << boost::this_thread::get_id() << " main thread id\n";
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
        //readyCond.wait(lock);
        Sleep(100);
    }
    lock.unlock();
    everyoneReady = true;
    std::cout << "wait ready\n";
    readyCond.notify_all();
    Sleep(1000);

}
