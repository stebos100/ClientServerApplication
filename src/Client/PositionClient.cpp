#include "PositionClient.h"
#include <iostream>
#include "../../include/Common.h"


PositionClient::PositionClient(const std::string& host, short port, const std::string& clientID, bool& debugLogs)
    : socket_(io_context_), running_(true), clientID_(clientID), clientDebugLogs_(debugLogs) {

    tcp::resolver resolver(io_context_);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    boost::asio::connect(socket_, endpoints);

}

PositionClient::~PositionClient() {

    stop();
    std::lock_guard<std::mutex> lock(print_mutex);
    std::cout << "Position client has been destructed...\n"; 
}

void PositionClient::start() {

    receive_thread_ = std::thread([this]() { 
        try {
            io_context_.run(); 
        } catch (const std::exception& e) {

            std::lock_guard<std::mutex> lock(print_mutex);
            std::cerr << "Exception in io_context.run(): " << e.what() << std::endl;
        }
    });

    std::thread(&PositionClient::do_receive, this).detach();
}

void PositionClient::stop() {

    if (!running_) return;

    running_ = false;

    socket_.close();
    io_context_.stop();

    if (receive_thread_.joinable()) {

        receive_thread_.join();
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << "Successfully joined client thread...\n";

    }
}

void PositionClient::handle_disconnection() {

    std::lock_guard<std::mutex> lock(print_mutex);
    std::cout << "\nServer closed connection or operation aborted" << std::endl;
    running_ = false;
    io_context_.stop();  
}

void PositionClient::send_position(const message_t& message) {

    auto msg = message;

    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    std::string timestamp_str = boost::posix_time::to_simple_string(now);
    std::copy(timestamp_str.begin(), timestamp_str.end(), msg.timestamp.begin());

    auto debug = clientDebugLogs_;

    auto buffer = std::make_shared<std::vector<char>>(reinterpret_cast<const char*>(&msg), reinterpret_cast<const char*>(&msg) + sizeof(msg));
    boost::asio::async_write(socket_, boost::asio::buffer(*buffer),
        [buffer](boost::system::error_code ec, std::size_t /*length*/) {

            if (ec) {
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cerr << "Failed to send message: " << ec.message() << std::endl;
            }
        });
}

void PositionClient::do_receive() {
    try {
        while (running_) {

            message_t message;
            
            boost::system::error_code error;
            size_t length = socket_.read_some(boost::asio::buffer(&message, sizeof(message_t)), error);

            if (error == boost::asio::error::eof || error == boost::asio::error::operation_aborted) {

                std::lock_guard<std::mutex> lock(print_mutex);
                handle_disconnection();
                break;

            } else if (error) {

                std::lock_guard<std::mutex> lock(print_mutex);
                handle_disconnection();
                break;
            }

            std::string symbol(message.symbol.data());

            if (symbol == clientID_) {
                continue;
            } else {

                std::lock_guard<std::mutex> lock(print_mutex);
                std::cout << "\nReceived broadcast on ClientID: " << clientID_ << "| Update for Client: " <<std::string(message.symbol.data()) << ", Net Position: " << message.net_position << ", Timestamp of update: " << std::string(message.timestamp.data()) << std::endl;

            }
        }
    } catch (const std::exception& e) {

        if (running_) {

            std::lock_guard<std::mutex> lock(print_mutex);
            std::cerr << "Exception in do_receive: " << e.what() << "\n";
            handle_disconnection();
        }
    }
}

