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

StreamingClient::StreamingClient(boost::asio::io_context& io_context, sc::tcp_client& network_client) : io_context_(io_context), network_client_(network_client) {}

void StreamingClient::start() {

    //boost::thread t1(boost::bind(&tcp_client::readThread, &network_client_));


    //boost::thread t2(boost::bind(&tcp_client::writeThread, &network_client_));

    displayModule_.run();
}

