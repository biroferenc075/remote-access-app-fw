#pragma once

#include "inputEvents.hpp"
#include "decompressionMod.hpp"
#include "consts.hpp"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lockfree/queue.hpp>
using boost::asio::ip::tcp;
using boost::lockfree::queue;

namespace rc {
    class tcp_client : public boost::enable_shared_from_this<tcp_client> {
    public:
        tcp_client(tcp::socket& socket, DecompressionModule& dm, bool& readyFlag, boost::condition_variable& readyCond, boost::mutex& mut);

        void readThread();

        // TODO event queue that the write thread processes with semaphor
        void writeThread();

        bool ready() {
            return isReady;
        }
        int findDelim(boost::asio::mutable_buffer& buf, int startIdx);

        void procFramebufferResize(int width, int height);
        void procKeypress(int key, int scancode, int action, int mods);
        void procMousePoll(double xpos, double ypos);
        void procMousePress(double xpos, double ypos, int button, int action, int mods);
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

        boost::interprocess::interprocess_semaphore sem;
        boost::lockfree::queue<InputEvent*> inputEventQueue{ size_t(64) };
    };
}



