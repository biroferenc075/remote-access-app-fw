#pragma once
#include "consts.hpp"
#include "frame.hpp"
#include "networkMod.hpp"
#include "displayMod.hpp"
#include <boost/lockfree/queue.hpp>
#include "BFE_image.hpp"


using namespace std;
using boost::asio::ip::tcp;
using boost::lockfree::queue;
using namespace BFE;

namespace sc {

    class StreamingClient {
    public:
        StreamingClient(boost::asio::io_context& io_context, tcp::socket&& socket, BFEWindow& bfeWindow, BFEDevice& bfeDevice);
        void start();
        void waitUntilReady();
        
    private:
        boost::asio::io_context& io_context_;
        tcp_client network_client_;
        DisplayModule displayModule_;
        DecompressionModule decompressionModule_;

        boost::mutex mut;

        boost::condition_variable readyCond;
        bool everyoneReady = false;

        bool shouldClose = false; // TODO shut down all threads

        BFEWindow& bfeWindow;
        BFEDevice& bfeDevice;
    };
}
