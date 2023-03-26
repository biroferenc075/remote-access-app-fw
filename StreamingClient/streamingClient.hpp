#pragma once

#include "frame.hpp"
#include "networkMod.hpp"
#include "displayMod.hpp"
#include <boost/lockfree/queue.hpp>


using namespace std;
using boost::asio::ip::tcp;
using boost::lockfree::queue;

namespace sc {

    class StreamingClient {
    public:
        StreamingClient(boost::asio::io_context& io_context, sc::tcp_client& network_client);
        void start();
    private:
        boost::asio::io_context& io_context_;
        tcp_client& network_client_;
        queue<Frame*>* imageQueue;
        DisplayModule displayModule_{};
    };
}
