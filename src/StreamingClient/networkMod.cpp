#include <iostream>
#include <string>

#include <boost/archive/text_oarchive.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>
#include "networkMod.hpp"

using boost::asio::ip::tcp;

using namespace sc;

const size_t imgsize = WIDTH * HEIGHT * 4;
const unsigned char delim[4] = { 1u,2u,3u,4u };
const int delimSize = 4;



tcp_client::tcp_client(tcp::socket& socket, DecompressionModule& dm, bool& readyFlag, boost::condition_variable& readyCond, boost::mutex& mut) : socket_(socket), dm(dm), everyoneReady(readyFlag), readyCond(readyCond), mut(mut), sem(0) {}
    void tcp_client::readThread() {
        try {

            isReady = true;

            boost::unique_lock<boost::mutex> lock(mut);
            while (!everyoneReady)
            {
                readyCond.wait(lock);
            }
            lock.unlock();
    
            boost::array<unsigned char, 1024> arr;
            boost::asio::mutable_buffer buf = boost::asio::buffer(arr);

            size_t vecSize = (imgsize + delimSize) * 2;
            size_t maxOffs = imgsize + delimSize;
            std::vector<unsigned char> imgarr(vecSize, 0);
            boost::asio::mutable_buffer imgbuf = boost::asio::buffer(imgarr);

            std::vector<unsigned char> cpyarr(vecSize, 0);
            boost::asio::mutable_buffer cpybuff = boost::asio::buffer(cpyarr);

            size_t len = 0;

        

            boost::asio::socket_base::receive_buffer_size option(2*(imgsize+delimSize));
            socket_.set_option(option);

            const char* msg = "r";
            socket_.send(boost::asio::buffer(msg, 2));

            size_t offs = 0;
            while (!shouldStop)
            {
                Frame* fr = new Frame(imgsize);

                int delim = -1;
                // read enough data for delim + img
                while (offs < maxOffs) {
                    boost::system::error_code error;
                    len = socket_.read_some(buf, error);
            
                    if (error == boost::asio::error::eof) {
                        std::cout << "found eof";
                        //shouldStop = true;

                        return; // Connection closed cleanly by peer.
                    }
                    else if (error) {
                        std::cout << "network error: " << error.message() << "\n";
                        shouldStop = true;
                    
                        return;
                        //throw boost::system::system_error(error); // Some other error.
                    }
                    memcpy((unsigned char*)imgbuf.data() + offs, buf.data(), len);
                    offs += len;
                }

                delim = findDelim(imgbuf, 0);
            
                // if both delimiters are found, and there is enough data between them, we can submit the frame
                if (delim != -1 && delim == imgsize) {
                    memcpy(fr->data, (const unsigned char*)imgbuf.data()+delimSize, imgsize);
                    dm.submitToQueue(fr);

                    memcpy(cpybuff.data(), (const unsigned char*)imgbuf.data() + delim + delimSize, offs - maxOffs);
                    fill(imgarr.begin(), imgarr.end(), 0);
                    memcpy(imgbuf.data(), (const unsigned char*)cpybuff.data(), offs-maxOffs);
                    offs = 0;
                }
                else {
                    // if we have found the delimiter but there is data lost before it, the data after it must be for the next frame, so we reuse it
                    if (delim != -1) {
                        memcpy(cpybuff.data(), (const unsigned char*)imgbuf.data() + delim + delimSize, offs - delim - delimSize);
                        fill(imgarr.begin(), imgarr.end(), 0);
                        memcpy(imgbuf.data(), (const unsigned char*)cpybuff.data(), offs - delim - delimSize);
                        offs = offs - delim - delimSize;
                    }
                    // if we haven't found the delimiter, there is nothing to do
                    else {
                        fill(imgarr.begin(), imgarr.end(), 0);
                        offs = 0;
                    }
                    delete fr; //TODO reuse previous frame instead of creating a new one if it was not sent
                }        
            }
        }
        catch (std::exception e) {
            std::cout << e.what();
        }
    }
    
    void tcp_client::writeThread() {
        
        while (true) {
            sem.wait();
            boost::asio::streambuf buf;
            std::ostream os(&buf);
            boost::archive::text_oarchive arch(os);

            InputEvent* ev;
            if (inputEventQueue.pop(ev)) { //TODO change into loop, but for some reason it breaks the serialization order
                switch (ev->getTypeId()) {
                case 0:
                    static_cast<MousePressEvent*>(ev)->serialize(arch, 0);
                    break;
                case 1:
                    static_cast<MousePollEvent*>(ev)->serialize(arch, 0);
                    break;
                case 2:
                    static_cast<KeyboardEvent*>(ev)->serialize(arch, 0);
                    break;
                case 3:
                    static_cast<WindowResizeEvent*>(ev)->serialize(arch, 0);
                    break;
              }   
              os << DELIM_CHAR;
            }

            //std::string s((std::istreambuf_iterator<char>(&buf)), std::istreambuf_iterator<char>());
            //std::cout << s << endl;

            //boost::asio::write(socket_, boost::asio::buffer(s.data(), strlen(s.data())));
            boost::asio::write(socket_, buf);
            delete ev;
        }
    }

    void tcp_client::handle_write(const boost::system::error_code& /*error*/,
        size_t /*bytes_transferred*/)
    {
    }

    int tcp_client::findDelim(boost::asio::mutable_buffer& buf, int startIdx) {
        auto d = (const unsigned char*)buf.data();
        for (int i = startIdx; i < buf.size(); i += delimSize) {

            bool found = true;
            for (int j = 0; j < delimSize; j++) {
                if (delim[j] != d[j+i]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return i;
            }
        }
        return -1;
    }

    void sc::tcp_client::procFramebufferResize(int width, int height)
    {
        //std::cout << "BUFF RESIZE";
        inputEventQueue.push(new WindowResizeEvent(width, height));
        sem.post();
    }

    void sc::tcp_client::procKeypress(int key, int scancode, int action, int mods)
    {
        if (action != GLFW_REPEAT) {
            //std::cout << "KEY PRESS";
            inputEventQueue.push(new KeyboardEvent(key, scancode, action, mods));
            sem.post();
        }
    }

    void sc::tcp_client::procMousePoll(double xpos, double ypos)
    {
        static double prevX = 0, prevY = 0;
        if (xpos != prevX || ypos != prevY) {
            //std::cout << "MOUSE POLL";
            inputEventQueue.push(new MousePollEvent(xpos, ypos));
            sem.post();
        }
        prevX = xpos;
        prevY = ypos;
    }

    void sc::tcp_client::procMousePress(double xpos, double ypos, int button, int action, int mods)
    {
        if (action != GLFW_REPEAT) {
            //std::cout << "MOUSE PRESS";
            inputEventQueue.push(new MousePressEvent(xpos, ypos, button, action, mods));
            sem.post();
        }
    }




