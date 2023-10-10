#pragma once

#include <ctime>
#include <iostream>
#include <string>
#include "consts.hpp"
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <stb_image.h>
#include "networkMod.hpp"
#include "inputEvents.hpp"

#include <boost/lockfree/queue.hpp>
using namespace BFE;

using boost::asio::ip::tcp;
using namespace BFE;

tcp_connection::pointer tcp_connection::create(boost::asio::io_context& io_context, bool& shouldStop, boost::function<void()> onDisconnectCallback)
{
    return pointer(new tcp_connection(io_context, shouldStop, onDisconnectCallback));
}

tcp::socket& tcp_connection::socket()
{
    return socket_;
}


void tcp_connection::start(boost::asio::io_context& io_context, boost::lockfree::queue<BFE::Frame*>& queue, boost::lockfree::queue<BFE::InputEvent*>& inputEventQueue)
{
    try {
        message_ = "make_daytime_string";
        /*for (int i = 0; i < frameNum; i++) {
            char buf[30];
            sprintf(buf, "frame_%02d_delay-0.1s.png", i);
            pixels[i] = stbi_load(buf, &imgWidth, &imgHeight, &imgChannels, STBI_rgb_alpha);
            if (!pixels[i]) {
                std::cout << buf;
                throw std::runtime_error("failed to load image!");
            }
        }*/
        //imageSize = imgWidth * imgHeight * 4;
        std::cout << imageSize << " imagesize \n";

        boost::thread t(boost::bind(&tcp_connection::readThread, this, boost::ref(inputEventQueue)));
        t.detach();
        writeThread(io_context, queue);
        
    }
    catch (std::exception e) {
        std::cout << e.what();
    }

}

void tcp_connection::readThread(boost::lockfree::queue<BFE::InputEvent*>& inputEventQueue) {
    //std::cout << "READ start" << std::endl;
    try {
        socket_.wait_read();
        while (!shouldStop)
        {
            //std::cout << "buf ";
            boost::asio::streambuf buf;
 
            boost::system::error_code error;

            size_t len = boost::asio::read_until(socket_, buf, DELIM_CHAR, error);
            //std::cout << "read " << len << " bytes";

            if (error == boost::asio::error::eof) {
                shouldStop = true;
                break; // Connection closed cleanly by peer.
            }
            else if (error) {
                shouldStop = true;
                throw boost::system::system_error(error); // Some other error.
            }

           // std::cout << "arch ";
            std::istream is(&buf);
            boost::archive::text_iarchive arch(is);

           // std::cout << "load ";
            try {
                InputEvent* ev = InputEvent::load(arch);

                //std::cout << "push ";
                inputEventQueue.push(ev);
            }
            catch (boost::archive::archive_exception e) {
                std::cout << "ouch" << e.what();
            }

            //std::cout << "wait ";
            socket_.wait_read();
        }
    }
    catch (std::exception e)
    {
        //std::cout << "AAAAAAAAAAAA\n";
        std::cout << e.what();
    }

}

void tcp_connection::writeThread(boost::asio::io_context& io_context, boost::lockfree::queue<BFE::Frame*>& queue) {
    socket_.wait_read();
    int l = 0;
    boost::array<char, 8> buf;
    while (l = socket_.read_some(boost::asio::buffer(buf))) {
        if (l > 0 && buf[0] == 'r') {
            break;
        }
    }
    boost::asio::steady_timer t(io_context, boost::asio::chrono::nanoseconds(1'000'000'000 / FRAMERATE));

    //std::cout << "write\n";
    int i = 0;
    while (!shouldStop)
    {
        BFE::Frame* f;
        if (queue.pop(f)) {
            imageSize = f->size;
            //sendMsg(pixels[i++ % frameNum]);
            sendMsg(f->data);
            sendDelim();
            //std::cout << i++ << " tick" << imageSize << " imagesize \n";
            t.expires_from_now(boost::asio::chrono::nanoseconds(1'000'000'000 / FRAMERATE));
            t.wait();

            delete f;
        }
    }
}

void tcp_connection::sendMsg(void* src) {
    int sent = 0;
    int total = 0;
    //boost::asio::async_write(socket_, boost::asio::buffer(src, imageSize), boost::bind(&tcp_connection::handle_write, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    while (total < imageSize) {
        sent = socket_.send(boost::asio::buffer((unsigned char*)src + total, imageSize - total));
        total += sent;
        //std::cout << imageSize - total << std::endl;
    }
}

void tcp_connection::sendDelim() {
    int sent = 0;
    int total = 0;
    while (total < delimSize) {
        sent = socket_.send(boost::asio::buffer(delim + total, delimSize - total));
        total += sent;
    }
}

tcp_connection::tcp_connection(boost::asio::io_context& io_context, bool& shouldStop, boost::function<void()> onDisconnectCallback)
    : socket_(io_context), shouldStop(shouldStop), onDisconnectCallback(onDisconnectCallback)
{
}

tcp_connection::~tcp_connection() {
    onDisconnectCallback();
}

void tcp_connection::handle_write(const boost::system::error_code& error,
    size_t bytes)
{
    std::cout << "message sent, bytes transferred: " << bytes << "\n";
    if (error) {
        std::cout << error.message();
    }
}


tcp_server::tcp_server(boost::asio::io_context& io_context, boost::lockfree::queue<BFE::Frame*>& frameQueue, boost::lockfree::queue<BFE::InputEvent*>& inputEventQueue, bool& shouldStop, boost::function<void()> onConnectCallback, boost::function<void()> onDisconnectCallback)
    : io_context_(io_context),
    acceptor_(io_context, tcp::endpoint(tcp::v4(), 13)), frameQueue(frameQueue), inputEventQueue(inputEventQueue), shouldStop(shouldStop),
    onConnectCallback(onConnectCallback), onDisconnectCallback(onDisconnectCallback)
{
    start_accept();
}

void tcp_server::start_accept()
{
    tcp_connection::pointer new_connection = tcp_connection::create(io_context_, shouldStop, onDisconnectCallback);

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection,
            boost::asio::placeholders::error));
}

void tcp_server::handle_accept(tcp_connection::pointer new_connection,
    const boost::system::error_code& error)
{
    if (!error)
    {
        shouldStop = false;
        onConnectCallback();
        new_connection->start(io_context_, frameQueue, inputEventQueue);
    }

    start_accept();
}
