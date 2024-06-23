#include "PositionClient.h"
#include <iostream>
#include "../../include/Common.h"


PositionClient::PositionClient(const std::string& host, short port, const std::string& clientID, bool& debugLogs)
    : host_(host), port_(port), socket_(std::make_unique<tcp::socket>(io_context_)), running_(false), 
        clientID_(clientID), clientDebugLogs_(debugLogs), buffer_(sizeof(message_t)), 
            work_guard_(boost::asio::make_work_guard(io_context_)) {

        std::cout << "Starting client...\n";

        receive_thread_ = std::make_unique<std::thread>([this]() {
        try {
            io_context_.run();

        } catch (const std::exception& e) {

            std::lock_guard<std::mutex> lock(print_mutex);
            std::cerr << "Exception in io_context.run(): " << e.what() << std::endl;
        }
    });

    connect();
}

PositionClient::~PositionClient() {

    stop();
    std::lock_guard<std::mutex> lock(print_mutex);
    std::cout << "Position client has been destructed...\n"; 
}

void PositionClient::stop() {

    if (!running_) return;

    running_ = false;

    boost::system::error_code ignore;

    if (socket_ && socket_->is_open()) {
        socket_->close();
    }
    
    {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << "\nSocket has been closed...\n";
    }

    io_context_.stop();

    {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << "\nIo service has been stopped...\n";
    }

    if (receive_thread_ && receive_thread_->joinable()) {
        receive_thread_->join();
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << "\nSuccessfully joined client thread...\n";
    }
}

void PositionClient::start() {

    if (receive_thread_ && receive_thread_->joinable()) {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << "Successfully joined recieve thread...\n";
    }

    std::cout << "We have entered the start function ...\n"; 

    receive_thread_ = std::make_unique<std::thread>([this]() {
        try {
            io_context_.run();

        } catch (const std::exception& e) {

            std::lock_guard<std::mutex> lock(print_mutex);
            std::cerr << "Exception in io_context.run(): " << e.what() << std::endl;
        }
    });

    std::thread(&PositionClient::do_receive, this).detach();

    std::cout << "We have exited the start function ...\n"; 

}

bool PositionClient::connect() {

    std::cout << "We have entered the connect function...\n";

    try {

        std::cout << "Attempting to connect...\n";

        socket_ = std::make_unique<tcp::socket>(io_context_);

        tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host_, std::to_string(port_));

        boost::system::error_code ec;
        boost::asio::connect(*socket_, endpoints, ec);

        std::cout << "connection call completed...\n";

        if (ec){
            std::cerr << "Connection failed: " << ec.message() << std::endl;
            socket_->close();
            return false;
        }
        else {

             {
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cout << "Connected to server.\n";
                running_ = true;
            }

            // Create and send a message_t structure
            message_t message;
            std::strncpy(message.symbol.data(), clientID_.data(), message.symbol.size());
            message.net_position = 123.45;

            boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
            std::string timestamp_str = boost::posix_time::to_simple_string(now);
            std::copy(timestamp_str.begin(), timestamp_str.end(), message.timestamp.begin());

            boost::system::error_code error;
            socket_->write_some(boost::asio::buffer(&message, sizeof(message_t)), error);

            do_receive();
            return true;
        }

    } catch (const std::exception& e) {

        std::lock_guard<std::mutex> lock(print_mutex);
        std::cerr << "Failed to connect: " << e.what() << std::endl;
        return false;
    }
}

void PositionClient::restart_io_context() {

    {
        std::lock_guard<std::mutex> lock(print_mutex);

        std::cout << "Restarting io_context...\n";
    }

    io_context_.restart();
    work_guard_.emplace(io_context_.get_executor());

}

void PositionClient::handle_disconnection() {

    {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << "\nServer closed connection or operation aborted" << std::endl;
        std::cout << "Attempting to disconnect...\n";
    }   
    
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

    running_ = false;
    handle_reconnect();
}

void PositionClient::handle_reconnect() {
    
    std::cout << "We have entered the Reconnect Handler ...\n"
              << "Restarting io_context...\n";

    io_context_.restart();
    work_guard_.emplace(io_context_.get_executor());

    std::this_thread::sleep_for(std::chrono::seconds(5));

    while (!running_) {

        std::cout << "Attempting to reconnect...\n";

        try {
            if (connect()) {
                break;
            }

        } catch (const std::exception& e) {

            std::cerr << "Reconnect attempt failed: " << e.what() << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void PositionClient::disconnect() {

    {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << "\nServer closed connection or operation aborted" << std::endl;
        std::cout << "Attempting to disconnect...\n";
    }   
    
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

    running_ = false;
}

void PositionClient::reconnect() {
    
    disconnect(); 
    std::this_thread::sleep_for(std::chrono::seconds(5));
    restart_io_context();
    connect();
}

void PositionClient::send_position(message_t& message) {

    if (!socket_->is_open() && running_) {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cerr << "Socket is not open. Cannot send message.\n";
        return;
    }

    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    std::string timestamp_str = boost::posix_time::to_simple_string(now);
    std::copy(timestamp_str.begin(), timestamp_str.end(), message.timestamp.begin());

    auto debug = clientDebugLogs_;

    std::memcpy(buffer_.data(), &message, sizeof(message));

    boost::asio::async_write(*socket_, boost::asio::buffer(buffer_, sizeof(message)),
        [this](boost::system::error_code ec, std::size_t /*length*/) {

            if (ec) {
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cerr << "Failed to send message: " << ec.message() << std::endl;
            }
        });
}

void PositionClient::do_receive() {

    if (!socket_->is_open()  || !running_) {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cerr << "Socket is not open. Cannot receive messages.\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return;
    }

    boost::asio::async_read(*socket_, boost::asio::buffer(&message_, sizeof(message_t)),
        [this](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                process_data(&message_, length);
                do_receive(); 

            } else {

                {
                    std::lock_guard<std::mutex> lock(print_mutex);
                    std::cerr << "Read error: " << ec.message() << std::endl;
                }

            // handle_disconnection();
        }
        });

}

void PositionClient::process_data(const message_t* message, std::size_t length) {

        std::lock_guard<std::mutex> lock(print_mutex);
        std::string symbol(message->symbol.data());
        if (symbol != clientID_) {
            std::cout << "\nReceived broadcast on ClientID: " << clientID_ << "| Update for Client: "
                      << symbol << ", Net Position: " << message->net_position
                      << ", Timestamp of update: " << std::string(message->timestamp.data()) << std::endl;
        }
    }

// void PositionClient::do_receive() {
//     try {
//         while (running_) {

//             message_t message;
            
//             boost::system::error_code error;
//             size_t length = socket_.read_some(boost::asio::buffer(&message, sizeof(message_t)), error);

//             if (error == boost::asio::error::eof || error == boost::asio::error::operation_aborted) {

//                 std::lock_guard<std::mutex> lock(print_mutex);
//                 handle_disconnection();
//                 break;

//             } else if (error) {

//                 std::lock_guard<std::mutex> lock(print_mutex);
//                 handle_disconnection();
//                 break;
//             }

//             std::string symbol(message.symbol.data());

//             if (symbol == clientID_) {
//                 continue;
//             } else {

//                 std::lock_guard<std::mutex> lock(print_mutex);
//                 std::cout << "\nReceived broadcast on ClientID: " << clientID_ << "| Update for Client: " <<std::string(message.symbol.data()) << ", Net Position: " << message.net_position << ", Timestamp of update: " << std::string(message.timestamp.data()) << std::endl;

//             }
//         }
//     } catch (const std::exception& e) {

//         if (running_) {

//             std::lock_guard<std::mutex> lock(print_mutex);
//             std::cerr << "Exception in do_receive: " << e.what() << "\n";
//             handle_disconnection();
//         }
//     }
// }

