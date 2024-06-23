#include <iostream>
#include <thread>
#include <chrono>
#include <boost/asio.hpp>
#include <memory>
#include <optional>

using boost::asio::ip::tcp;

// Server class
class Server {
public:
    Server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), io_context_(io_context) {
        start_accept();
    }

private:
    void start_accept() {
        auto socket = std::make_shared<tcp::socket>(io_context_);
        acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
            if (!ec) {
                std::cout << "Accepted connection from: " << socket->remote_endpoint().address().to_string() << "\n";
                start_accept();  // Accept the next connection
            } else {
                std::cerr << "Accept failed: " << ec.message() << "\n";
            }
        });
    }

    tcp::acceptor acceptor_;
    boost::asio::io_context& io_context_;
};

// Client class
class Client {
public:
    Client(const std::string& host, short port)
        : host_(host), port_(port), socket_(nullptr), work_guard_(boost::asio::make_work_guard(io_context_)) {}

    void start() {
        std::cout << "Starting client...\n";
        io_context_thread_ = std::thread([this]() { io_context_.run(); });
        connect();
    }

    void stop() {
        std::cout << "Stopping client...\n";
        work_guard_.reset();
        io_context_.stop();
        if (io_context_thread_.joinable()) {
            io_context_thread_.join();
        }
    }

    void restart_io_context() {
        std::cout << "Restarting io_context...\n";
        io_context_.restart();
        work_guard_.emplace(io_context_.get_executor());
    }

    bool connect() {
        std::cout << "Attempting to connect...\n";
        socket_ = std::make_unique<tcp::socket>(io_context_);
        tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host_, std::to_string(port_));
        boost::system::error_code ec;
        boost::asio::connect(*socket_, endpoints, ec);

        if (ec) {
            std::cerr << "Connection failed: " << ec.message() << "\n";
            socket_->close();
            return false;
        } else {
            std::cout << "Connected to server.\n";
            start_read_message();
            return true;
        }
    }

    void disconnect() {
        std::cout << "Attempting to disconnect...\n";
        if (socket_ && socket_->is_open()) {
            boost::system::error_code ec;
            socket_->shutdown(tcp::socket::shutdown_both, ec);
            socket_->close(ec);
            if (!ec) {
                std::cout << "Disconnected from server.\n";
            } else {
                std::cerr << "Disconnection failed: " << ec.message() << "\n";
            }
        }
    }

    void retry_connect() {
        std::cout << "Attempting to retry connection...\n";
        disconnect();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        restart_io_context();
        connect();
    }

private:
    void start_read_message() {
        auto buffer = std::make_shared<std::vector<char>>(1024);
        socket_->async_receive(boost::asio::buffer(*buffer), [this, buffer](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::string message(buffer->data(), length);
                std::cout << "Received: " << message << std::endl;
                start_read_message();
            } else {
                std::cerr << "Receive failed: " << ec.message() << "\n";
            }
        });
    }

    std::string host_;
    short port_;
    boost::asio::io_context io_context_;
    std::unique_ptr<tcp::socket> socket_;
    std::optional<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> work_guard_;
    std::thread io_context_thread_;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <host>\n";
        return 1;
    }

    short port = std::atoi(argv[1]);
    std::string host = argv[2];

    boost::asio::io_context io_context;

    // Start the server in a separate thread
    std::thread server_thread([&io_context, port]() {
        Server server(io_context, port);
        io_context.run();
    });

    // Allow the server to start
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Start the client
    Client client(host, port);
    client.start();

    // Simulate some activity
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Disconnect from the server
    client.disconnect();

    // Simulate waiting before reconnecting
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Reconnect to the server
    client.retry_connect();

    // Allow the client to perform its operations
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // Stop the client
    client.stop();

    // Stop the server
    io_context.stop();
    server_thread.join();

    return 0;
}
