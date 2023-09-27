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

#include <stb_image.h>
#include "StreamingServer.hpp"

#include <boost/lockfree/queue.hpp>

using boost::asio::ip::tcp;

tcp_connection::pointer tcp_connection::create(boost::asio::io_context& io_context, bool& shouldStop)
{
    return pointer(new tcp_connection(io_context, shouldStop));
}

tcp::socket& tcp_connection::socket()
{
    return socket_;
}


void tcp_connection::start(boost::asio::io_context& io_context, boost::lockfree::queue<BFE::Frame*>& queue)
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

        boost::thread t(boost::bind(&tcp_connection::readThread, this));
        t.detach();
        writeThread(io_context, queue);
    }
    catch (std::exception e) {
        std::cout << e.what();
    }

}

void tcp_connection::readThread() {
    try {
        while (!shouldStop)
        {
            boost::array<char, 128> buf;
            boost::system::error_code error;

            size_t len = socket_.read_some(boost::asio::buffer(buf), error);

            if (error == boost::asio::error::eof) {
                shouldStop = true;
                break; // Connection closed cleanly by peer.
            }
            else if (error) {
                shouldStop = true;
                throw boost::system::system_error(error); // Some other error.
            }

            message_.assign(buf.data(), len);
        }
    }
    catch (std::exception e)
    {
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
    boost::asio::steady_timer t(io_context, boost::asio::chrono::nanoseconds(1'000'000'000 / FRAMERATE));//boost::asio::chrono::milliseconds(250)); //boost::asio::chrono::nanoseconds(1'000'000'000 / 30));

    std::cout << "write\n";
    int i = 0;
    while (!shouldStop)
    {
        BFE::Frame* f;
        if (queue.pop(f)) {
            imageSize = f->size;
            //sendMsg(pixels[i++ % frameNum]);
            sendMsg(f->data);
            sendDelim();
            std::cout << i++ << " tick" << imageSize << " imagesize \n";
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

tcp_connection::tcp_connection(boost::asio::io_context& io_context, bool& shouldStop)
    : socket_(io_context), shouldStop(shouldStop)
{
}

void tcp_connection::handle_write(const boost::system::error_code& error,
    size_t bytes)
{
    std::cout << "message sent, bytes transferred: " << bytes << "\n";
    if (error) {
        std::cout << error.message();
    }
}


tcp_server::tcp_server(boost::asio::io_context& io_context, boost::lockfree::queue<BFE::Frame*>& queue, bool& shouldStop)
    : io_context_(io_context),
    acceptor_(io_context, tcp::endpoint(tcp::v4(), 13)), queue(queue), shouldStop(shouldStop)
{
    start_accept();
}
void tcp_server::registerConnectCallback(boost::function<void()> onConnectCallback) {
    this->onConnectCallback = onConnectCallback;
}
void tcp_server::start_accept()
{
    tcp_connection::pointer new_connection = tcp_connection::create(io_context_, shouldStop);

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection,
            boost::asio::placeholders::error));
}

void tcp_server::handle_accept(tcp_connection::pointer new_connection,
    const boost::system::error_code& error)
{
    if (!error)
    {
        onConnectCallback();
        new_connection->start(io_context_, queue);
    }

    start_accept();
}
