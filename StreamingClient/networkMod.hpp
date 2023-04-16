#pragma once

#include "decompressionMod.hpp"

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lockfree/queue.hpp>
using boost::asio::ip::tcp;
using boost::lockfree::queue;

namespace sc {
    class tcp_client : public boost::enable_shared_from_this<tcp_client> {
    public:
        tcp_client(tcp::socket& socket, DecompressionModule& dm, bool& readyFlag, boost::condition_variable& readyCond, boost::mutex& mut);

        void readThread();

        void writeThread();

        bool ready() {
            return isReady;
        }
    private:
        tcp::socket& socket_;

        void handle_write(const boost::system::error_code& /*error*/,
            size_t /*bytes_transferred*/);

        bool shouldStop = false;
        DecompressionModule& dm;

        bool& everyoneReady;
        bool isReady = false;
        boost::condition_variable& readyCond;
        boost::mutex& mut;
        queue<BFEImage* > imageQueue;
    };
}



