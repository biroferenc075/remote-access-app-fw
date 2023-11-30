#pragma once
#include "consts.hpp"
#include "frame.hpp"
#include "networkMod.hpp"
#include "displayMod.hpp"
#include <boost/lockfree/queue.hpp>
#include "../RAC-Core/RAC_image.hpp"


using namespace std;
using boost::asio::ip::tcp;
using boost::lockfree::queue;
using namespace RAC;

namespace rc {

    class RemoteClient {
    public:
        RemoteClient(boost::asio::io_context& io_context, tcp::socket&& socket, RACWindow& racWindow, RACDevice& racDevice);
        void start();
        void waitUntilReady();
        void registerCallbacks(RACWindow& window);
        void pollMouse();
    private:
        boost::asio::io_context& io_context_;
        tcp_client network_client_;
        DisplayModule displayModule_;
        DecompressionModule decompressionModule_;

        boost::mutex mut;

        boost::condition_variable readyCond;
        bool everyoneReady = false;

        bool shouldClose = false; // TODO shut down all threads

        RACWindow& racWindow;
        RACDevice& racDevice;

        boost::asio::deadline_timer mousePollTimer = boost::asio::deadline_timer(io_context_);
    };
}
