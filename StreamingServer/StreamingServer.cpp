
#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>

using boost::asio::ip::tcp;

bool shouldStop = false;
std::string make_daytime_string()
{
    using namespace std;
    time_t now = time(0);
#pragma warning(disable : 4996)
    return ctime(&now);
}

class tcp_connection
    : public boost::enable_shared_from_this<tcp_connection>
{
public:
    typedef boost::shared_ptr<tcp_connection> pointer;

    static pointer create(boost::asio::io_context& io_context)
    {
        return pointer(new tcp_connection(io_context));
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start(boost::asio::io_context& io_context)
    {
        message_ = make_daytime_string();

        boost::thread t(boost::bind(&tcp_connection::readThread, this));
        writeThread(io_context);
    }

    void readThread() {
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

    void writeThread(boost::asio::io_context& io_context) {
        boost::asio::steady_timer t(io_context, boost::asio::chrono::seconds(1));

        while (!shouldStop) {
            t.wait();
            t.expires_from_now(boost::asio::chrono::seconds(1));
            sendMsg();
        }
    }

    void sendMsg() {
        boost::asio::async_write(socket_, boost::asio::buffer(message_),
            boost::bind(&tcp_connection::handle_write, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));

    }

private:
    tcp_connection(boost::asio::io_context& io_context)
        : socket_(io_context)
    {
    }

    void handle_write(const boost::system::error_code& /*error*/,
        size_t /*bytes_transferred*/)
    {
    }

    tcp::socket socket_;
    std::string message_;
};

class tcp_server
{
public:
    tcp_server(boost::asio::io_context& io_context)
        : io_context_(io_context),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), 13))
    {
        start_accept();
    }

private:
    void start_accept()
    {
        tcp_connection::pointer new_connection =
            tcp_connection::create(io_context_);

        acceptor_.async_accept(new_connection->socket(),
            boost::bind(&tcp_server::handle_accept, this, new_connection,
                boost::asio::placeholders::error));
    }

    void handle_accept(tcp_connection::pointer new_connection,
        const boost::system::error_code& error)
    {
        if (!error)
        {
            new_connection->start(io_context_);
        }

        start_accept();
    }

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};

int main()
{
    try
    {
        boost::asio::io_context io_context;
        tcp_server server(io_context);
        boost::thread t(boost::bind(&boost::asio::io_service::run, &io_context));
        char inp;
        do {
            std::cin >> inp;
        } while (inp != 'x');
        shouldStop = true;
        io_context.stop();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}