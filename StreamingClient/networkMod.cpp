#include <iostream>
#include <string>

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>
#include "networkMod.hpp"

using boost::asio::ip::tcp;

using namespace sc;

const size_t imgsize = 600 * 800 * 4;

tcp_client::tcp_client(tcp::socket& socket, DecompressionModule& dm, bool& readyFlag, boost::condition_variable& readyCond, boost::mutex& mut) : socket_(socket), dm(dm), everyoneReady(readyFlag), readyCond(readyCond), mut(mut) {}
    void tcp_client::readThread() {
        std::cout << "netw read init\n";
        isReady = true;

        boost::unique_lock<boost::mutex> lock(mut);
        while (!everyoneReady)
        {
            std::cout << everyoneReady;
            readyCond.wait(lock);
        }
        lock.unlock();
        std::cout << "netw read start\n";
    
        boost::array<unsigned char, 1024> buf;
        //std::cout << "buf data " << reinterpret_cast<void*>(buf.data()) << endl;
        size_t len = 0;
        size_t remainder = 0;

        while (!shouldStop)
        {
            size_t offs = 0;
            Frame* fr = new Frame(imgsize);
            //std::cout << "netw memcpy\n";
            memcpy(fr->data, buf.data() + len - remainder, remainder);
        
            while (offs < imgsize) {
                boost::system::error_code error;

               // std::cout << "netw read_some\n";
                len = socket_.read_some(boost::asio::buffer(buf), error);
            
                if (error == boost::asio::error::eof) {
                    std::cout << "found eof";
                    //shouldStop = true;

                    break; // Connection closed cleanly by peer.
                }
                else if (error) {
                    std::cout << "network error: " << error.message() << "\n";
                    shouldStop = true;
                    //Sleep(60000);
                    return;
                    //throw boost::system::system_error(error); // Some other error.
                }

                long long res = offs + len - static_cast<signed long long>(imgsize);
                //std::cout << "res: " << res << "\n";
                if (res > 0) {
                    remainder = res;
                }
                else {
                    remainder = 0;
                }
                //remainder = offs + len - imgsize;
                //std::cout << "netw offs: " << offs << " len: " << len <<" remainder: " << remainder << "\n";
                if (remainder > 0) {
                    //std::cout << "memcpy rem>0\n";
                    memcpy(fr->data + offs, buf.data(), len-remainder);
                }
                else {
                    //unsigned char* p1 = static_cast<unsigned char*>(buf.data());
                    //std::cout << "memcpy rem " << p1[0] << p1[1] << p1[2] << "xd" << endl;
                    memcpy(fr->data + offs, buf.data(), len);
                }

           

                offs += len;
                //std::cout << "netw offs: " << offs << " len: " << len << "\n";
            }
            //std::cout << "netw submittoqueue\n";
            dm.submitToQueue(fr);
            std::cout << "submitted frame to decompress\n";
            //Sleep(5000);
            std::cout << "frame size: " << fr->size << " data: \n";
            //std::cout << std::hex << std::setfill('0');  // needs to be set only once
            //auto* ptr = reinterpret_cast<unsigned char*>(fr->data);
            //for (int i = 0; i < (int)fr->size; i++, ptr++) {
            //    std::cout << std::setw(2) << static_cast<unsigned>(*ptr);
            //}
        
        }
        std::cout << "netw ended\n";
    }

    void tcp_client::writeThread() {
        std::string inp;
        do {
            std::cin >> inp;

            boost::asio::async_write(socket_, boost::asio::buffer(inp), 
                boost::bind(&tcp_client::handle_write, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
        } while (inp.at(0) != 'x' && inp.length() == 1 && !shouldStop);
        shouldStop = true;
    }

    void tcp_client::handle_write(const boost::system::error_code& /*error*/,
        size_t /*bytes_transferred*/)
    {
    }




