#include <iostream>
#include <string>

#include "networkMod.hpp"
#include "RemoteClient.hpp"

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>
using namespace std;
using namespace rc;

using boost::asio::ip::tcp;

rc::RemoteClient::RemoteClient(boost::asio::io_context& io_context, tcp::socket&& socket, RACWindow& racWindow, RACDevice& racDevice) : 
io_context_(io_context), racWindow(racWindow), racDevice(racDevice),
network_client_(socket, decompressionModule_, everyoneReady, readyCond, mut), 
displayModule_(io_context, everyoneReady, readyCond, mut, racWindow, racDevice),
decompressionModule_(everyoneReady, readyCond, mut, displayModule_, racDevice) {
}

void RemoteClient::start() {
    try {
    boost::thread networkRead(boost::bind(&tcp_client::readThread, &network_client_));
    boost::thread networkWrite(boost::bind(&tcp_client::writeThread, &network_client_));

    boost::thread decomp(boost::bind(&DecompressionModule::run, &decompressionModule_));

    networkRead.detach();
    networkWrite.detach();
    decomp.detach();

    boost::thread ready(boost::bind(&RemoteClient::waitUntilReady, this));
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> guard = boost::asio::make_work_guard(io_context_);

    boost::thread io(boost::bind(&boost::asio::io_context::run, &io_context_));
    io.detach();

    displayModule_.run();
    }
    catch (std::exception e) {
        std::cout << e.what();
    }
}

void RemoteClient::waitUntilReady() {
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

void rc::RemoteClient::registerCallbacks(RACWindow& window)
{
    using namespace boost;
    
    window.registerCallbacks(
        bind(&tcp_client::procFramebufferResize, &network_client_, boost::placeholders::_1, boost::placeholders::_2),
        bind(&tcp_client::procKeypress, &network_client_, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4),
        bind(&tcp_client::procMousePress, &network_client_, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5)
    );

    mousePollTimer.expires_from_now(boost::posix_time::milliseconds(MOUSE_POLL_RATE));
    mousePollTimer.async_wait(bind(&RemoteClient::pollMouse, this));
}

void rc::RemoteClient::pollMouse() {
    double x, y;
    glfwGetCursorPos(racWindow.getGLFWwindow(), &x, &y);
    network_client_.procMousePoll(x, y);
   

    mousePollTimer.expires_from_now(boost::posix_time::milliseconds(MOUSE_POLL_RATE));
    mousePollTimer.async_wait(bind(&RemoteClient::pollMouse, this));
}
