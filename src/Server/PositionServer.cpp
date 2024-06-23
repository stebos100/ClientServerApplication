#include "PositionServer.h"
#include "../../include/Common.h"
#include <iostream>

PositionServer::PositionServer(short port, bool& debugLogs)
    : port_(port), acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)),
      message_queue_(1024),
      running_(false),
      debugLogs_(debugLogs) {}

PositionServer::~PositionServer() {
    stop();
}

void PositionServer::stop() {

    std::cout << "Stopping server..." << std::endl;
    running_ = false;

    boost::system::error_code ec;
    acceptor_.close(ec);
    if (ec) {

        std::cerr << "Error closing acceptor: " << ec.message() << std::endl;
    }

    io_context_.stop();

    if (accept_thread_.joinable() && debugLogs_) {

        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << "Joining worker thread..." << std::endl;
        accept_thread_.join();
    }
    else if( accept_thread_.joinable()){

        accept_thread_.join();
    }

    for (auto& thread : worker_threads_) {

        if (thread.joinable() && debugLogs_) {

            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << "Joining worker thread..." << std::endl;
            thread.join();
        }
        else if( thread.joinable()){

            thread.join();
        }
    }

    {                    
        clients_.clear();
        connected_client_ids_.clear(); 
        client_positions_.clear(); 

        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << "Server resources cleaned up and stopped." << std::endl;

    }

}

void PositionServer::start() {

    if (running_) {

        std::lock_guard<std::mutex> lock(print_mutex); 
        std::cout << "Server is already running." << std::endl;
        return;
    }

    running_ = true;
    
    std::cout << "Starting PositionServer on port " << port_ << std::endl;

    accept_thread_ = std::thread([this]() {
        do_accept();
        io_context_.run();
    });

    std::cout << "Starting worker threads" << std::endl;
    for (size_t i = 0; i < 2; ++i) {
        worker_threads_.emplace_back(&PositionServer::process_messages, this);
    }

    std::cout << "Running io_context" << std::endl;
}

void PositionServer::do_accept() {

    auto socket = std::make_shared<tcp::socket>(io_context_);

    acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
        if (!ec) {

            message_t message;
            boost::system::error_code error;
            size_t length = socket->read_some(boost::asio::buffer(&message, sizeof(message_t)), error);

            if (error) {
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cerr << "Failed to read client ID: " << error.message() << std::endl;
                return;
            }

            std::string received_symbol(message.symbol.data(), strnlen(message.symbol.data(), message.symbol.size()));
            std::string received_timestamp(message.timestamp.data(), strnlen(message.timestamp.data(), message.timestamp.size()));

            {
                std::lock_guard<std::mutex> lock(clients_mutex_);
                if (connected_client_ids_.find(received_symbol) != connected_client_ids_.end()) {

                    std::lock_guard<std::mutex> lock(print_mutex);
                    std::cerr << "Client ID " << received_symbol << " already exists. Rejecting connection." << std::endl;
                    socket->close();
                    return;

                } else {

                    connected_client_ids_.insert(received_symbol);
                    clients_.insert(socket);
                }
            }

            {
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cout << "Accepted connection from: " << socket->remote_endpoint().address().to_string() << " with Client ID: " << received_symbol << std::endl;
            }
            
            std::thread(&PositionServer::handle_client, this, socket, received_symbol).detach();
            do_accept();

        } else {
            if (running_) {
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cerr << "Accept error: " << ec.message() << std::endl;
            }
        }

    });
}

void PositionServer::simulate_disconnect() {

    stop();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    try {

        acceptor_.open(tcp::v4());
        acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
        acceptor_.bind(tcp::endpoint(tcp::v4(), port_));
        acceptor_.listen();
        start();

    } catch (const boost::system::system_error& e) {

        std::cerr << "Failed to start the acceptor: " << e.what() << std::endl;
        stop(); 
        return;
    }
}


void PositionServer::handle_client(std::shared_ptr<tcp::socket> socket, const std::string& client_id) {
    try {
        while (running_) {

            message_t message;
            boost::system::error_code error;
            size_t length = socket->read_some(boost::asio::buffer(&message, sizeof(message_t)), error);

            if (error) {
                if (error == boost::asio::error::eof) {

                    std::lock_guard<std::mutex> lock(print_mutex);
                    std::cout << "Client " << client_id << " closed connection.\n" << std::endl;
                    break;
                } else if (error == boost::asio::error::operation_aborted) {

                    std::lock_guard<std::mutex> lock(print_mutex);
                    std::cout << "Operation aborted for client " << client_id << ".\n" << std::endl;
                    break;
                } else {

                    std::lock_guard<std::mutex> lock(print_mutex);
                    std::cerr << "Error in handle_client for " << client_id << ": " << error.message() << std::endl;
                    break;
                }
            }

            std::string symbol(message.symbol.data());
            
            client_positions_[symbol] = message;
            connected_client_ids_.insert(client_id);
            enqueue_message(message);

        }
    } catch (const std::exception& e) {
        if (running_) {

            std::lock_guard<std::mutex> lock(print_mutex);
            std::cerr << "Exception in handle_client for " << client_id << ": " << e.what() << "\n";
        }
    }

    {
        std::scoped_lock lock(clients_mutex_, print_mutex);
        clients_.erase(socket);
        connected_client_ids_.erase(client_id);
        std::cout << "Client " << client_id << " disconnected and removed from the set." << std::endl;

    }
}

void PositionServer::enqueue_message(const message_t& message) {

    while (!message_queue_.push(message)) {
        std::this_thread::yield();
    }
}

void PositionServer::process_messages() {

    while (running_) {

        message_t message;

        while (message_queue_.pop(message)) {

            std::lock_guard<std::mutex> lock(clients_mutex_);

            for (auto& client : clients_) {

                if (client->is_open()) {

                    auto buffer = std::make_shared<std::vector<char>>(reinterpret_cast<const char*>(&message), reinterpret_cast<const char*>(&message) + sizeof(message));

                    boost::asio::async_write(*client, boost::asio::buffer(*buffer),
                        [this, client, buffer, message](boost::system::error_code ec, std::size_t length) {
                            if (ec) {

                                std::cerr << "Failed to send message: " << ec.message() << std::endl;
                                clients_.erase(client); 

                            } 
                            else if (debugLogs_) {
                                
                                std::lock_guard<std::mutex> lock(print_mutex);                          
                                std::cout << "\nSent broadcast: Client: " << std::string(message.symbol.data()) << ", Net Position: " << message.net_position << ", Timestamp of client: " << std::string(message.timestamp.data()) << std::endl;
                            }
                        });
                }
            }
        }
        std::this_thread::yield();
    }
}