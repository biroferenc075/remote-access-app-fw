#pragma once

#include <ctime>
#include <iostream>
#include <string>
#include <boost/lockfree/lockfree_forward.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>

#include <stb_image.h>

#include "consts.hpp"
#include "frame.hpp"
#include "inputEvents.hpp"

using boost::asio::ip::tcp;
using boost::lockfree::queue;



class tcp_connection
    : public boost::enable_shared_from_this<tcp_connection>
{
public:
    typedef boost::shared_ptr<tcp_connection> pointer;

    static pointer create(boost::asio::io_context& io_context, bool& shouldStop, boost::function<void()> onDisconnectCallback);
    tcp::socket& socket();

    static const int frameNum = 57;
    unsigned char* pixels[frameNum];
    int imgWidth = 0, imgHeight = 0, imgChannels = 0, imageSize = 0;
    unsigned char delim[4] = { 1u,2u,3u,4u };
    int delimSize = 4;

    void start(boost::asio::io_context& io_context, queue<BFE::Frame*>& frameQueue, boost::lockfree::queue<BFE::InputEvent*>& inputEventQueue);
    void readThread(boost::lockfree::queue<BFE::InputEvent*>& inputEventQueue);
    void writeThread(boost::asio::io_context& io_context, boost::lockfree::queue<BFE::Frame*>& queue);

    void sendMsg(void* src);

    void sendDelim();
    ~tcp_connection();
private:
    bool& shouldStop;
    tcp_connection(boost::asio::io_context& io_context, bool& shouldStop, boost::function<void()> onDisconnectCallback);

    void handle_write(const boost::system::error_code& error, size_t bytes);

    tcp::socket socket_;
    std::string message_;
    boost::function<void()> onDisconnectCallback;
};

class tcp_server
{
public:
    tcp_server(boost::asio::io_context& io_context, boost::lockfree::queue<BFE::Frame*>& frameQueue, boost::lockfree::queue<BFE::InputEvent*>& inputEventQueue, bool& shouldStop, boost::function<void()> onConnectCallback, boost::function<void()> onDisconnectCallback);
private:
    bool& shouldStop;
    boost::lockfree::queue<BFE::Frame*>& frameQueue;
    boost::lockfree::queue<BFE::InputEvent*>& inputEventQueue;
    boost::function<void()> onConnectCallback;
    boost::function<void()> onDisconnectCallback;
    void start_accept();

    void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error);

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};