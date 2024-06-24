#include "PositionServer.h"
#include "../../include/Common.h"
#include <iostream>

PositionServer::PositionServer(short port, bool& debugLogs)
    : port_(port), acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)),
      message_queue_(1024),
      running_(false),
      debugLogs_(debugLogs),
      buffer_(sizeof(message_t)) {
        
        std::lock_guard<std::mutex> lock(print_mutex); 
        std::cout << "PositionServer constructed and acceptor initialized on port " << port << std::endl;

      }

PositionServer::~PositionServer() {
    stop();
}

void PositionServer::stop() {

    if (!running_) {
        std::lock_guard<std::mutex> lock(print_mutex); 
        std::cout << "Server is not running." << std::endl;
        return;
    }

    running_ = false;
    
    std::lock_guard<std::mutex> lock(print_mutex);
    std::cout << "Stopping server..." << std::endl;

    boost::system::error_code ec;
    acceptor_.cancel(ec);
    acceptor_.close(ec);

    io_context_.stop();

    std::cout << "Stopping server and closing all client connections...\n";
    for (auto& client : clients_) {
        client->close();
    }
    
    clients_.clear();
    connected_client_ids_.clear();

    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }

    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    worker_threads_.clear();

    std::cout << "Server stopped." << std::endl;
}

void PositionServer::start() {

    if (running_) {
        std::lock_guard<std::mutex> lock(print_mutex); 
        std::cout << "Server is already running." << std::endl;
        return;
    }

    running_ = true;
    
    std::cout << "Starting PositionServer on port " << port_ << std::endl;

    if (!acceptor_.is_open()) {
        std::lock_guard<std::mutex> lock(print_mutex); 
        std::cerr << "Acceptor is not open." << std::endl;
        stop(); 
        return;
    }

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

            {
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cout << "Accepted connection from: " << socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port() << std::endl;
            }

            start_read(socket);

            do_accept();
        } else {

            if (running_) {
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cerr << "Accept error: " << ec.message() << std::endl;
            }
        }
    });
}

void PositionServer::sendPositions(const std::string& clientId, std::shared_ptr<tcp::socket> socket) {
 
    std::lock_guard<std::mutex> lock(clients_mutex_);

            for (auto& client : client_positions_) {

                if(client.first == clientId) {
                    continue;
                }
                else {

                    auto & msg = client.second;
                    
                    boost::asio::async_write(*socket, boost::asio::buffer(&client.second, sizeof(message_t)),
                        [this, msg , socket, clientId](boost::system::error_code ec, std::size_t length) {
                            if (ec) {

                                std::cerr << "Failed to send message: " << ec.message() << std::endl;
                            } 
                            else {

                                std::lock_guard<std::mutex> lock(print_mutex);                
                                std::cout << "Sending BroadCast to: " << socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port() << std::endl;
                                std::cout << "\nSent broadcast: Client positions to (" << clientId << ") upon joining:|\t " << std::string(msg.symbol.data()) << ", Net Position: " << msg.net_position << ", Timestamp of client: " << std::string(msg.timestamp.data()) << std::endl;
                            }
                        });

                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
}


void PositionServer::start_read(std::shared_ptr<tcp::socket> socket) {

    auto message = std::make_shared<message_t>();

    boost::asio::async_read(*socket, boost::asio::buffer(message.get(), sizeof(message_t)),
        [this, socket, acceptMessage_ = message](boost::system::error_code ec, std::size_t length) {
            if (!ec) {

                std::string received_symbol(acceptMessage_->symbol.data(), strnlen(acceptMessage_->symbol.data(), acceptMessage_->symbol.size()));
                std::string received_timestamp(acceptMessage_->timestamp.data(), strnlen(acceptMessage_->timestamp.data(), acceptMessage_->timestamp.size()));
                double received_net_position = acceptMessage_->net_position;

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
                    std::cout << "Received message from client: " << received_symbol << ", net position: " << received_net_position << ", timestamp: " << received_timestamp << std::endl;
                }

                sendPositions(received_symbol, socket);

                std::thread(&PositionServer::handle_client, this, socket, received_symbol).detach();

            } else {
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cerr << "Read error: " << ec.message() << std::endl;
            }
        });
}

void PositionServer::simulate_disconnect() {

    stop();

    std::this_thread::sleep_for(std::chrono::seconds(1));

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
            boost::system::error_code ec;
            size_t length = socket->read_some(boost::asio::buffer(&message, sizeof(message_t)), ec);

            if (ec) {
                std::lock_guard<std::mutex> lock(print_mutex);
                if (ec == boost::asio::error::eof) {
                    std::cout << "Client " << client_id << " closed connection.\n";
                } else if (ec == boost::asio::error::operation_aborted) {
                    std::cout << "Operation aborted for client " << client_id << ".\n";
                } else {
                    std::cerr << "Error in handle_client for " << client_id << ": " << ec.message() << std::endl;
                }
                handle_disconnection(socket, client_id);
                break;
            }

            process_data(socket, message);
        }
    } catch (const std::exception& e) {
        if (running_) {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cerr << "Exception in handle_client for " << client_id << ": " << e.what() << "\n";
            handle_disconnection(socket, client_id);
        }
    }
}

void PositionServer::handle_disconnection(std::shared_ptr<tcp::socket> socket, const std::string& clients_ID) {

    std::lock_guard<std::mutex> lock(clients_mutex_);

    auto it = clients_.find(socket);
    auto itTwo = connected_client_ids_.find(clients_ID); 

    if (it != clients_.end()) {

        std::string IPID = (*it)->remote_endpoint().address().to_string();
        boost::system::error_code ec;

        (*it)->close(ec);
        if (ec) {
            std::cerr << "Error during socket close: " << ec.message() << std::endl;
        }

        clients_.erase(it);
        std::cout << "Client " << IPID << " disconnected and removed from the set." << std::endl;
    } else {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cerr << "Client socket not found in the set." << std::endl;
    }

    if (itTwo != connected_client_ids_.end()) {

        connected_client_ids_.erase(clients_ID);
        std::cout << "Client " << clients_ID << " disconnected and removed from the set." << std::endl;

    } else {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cerr << "Client ID not found in the set." << std::endl;

    }
}

void PositionServer::process_data(std::shared_ptr<tcp::socket> socket, message_t& message) {

    std::string symbol(message.symbol.data());

    if(debugLogs_) 
    {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << "Processing data for client: " << symbol << std::endl;
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        client_positions_[symbol] = message;
    }

    enqueue_message(message);
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

                    boost::asio::async_write(*client, boost::asio::buffer(&message, sizeof(message_t)),
                        [this, client, message](boost::system::error_code ec, std::size_t length) {
                            if (ec) {

                                std::cerr << "Failed to send message: " << ec.message() << std::endl;
                                clients_.erase(client); 

                            } 
                            else if(debugLogs_){

                                std::lock_guard<std::mutex> lock(print_mutex);                
                                std::cout << "Sending BroadCast to: " << client->remote_endpoint().address().to_string() << ":" << client->remote_endpoint().port() << std::endl;
                                std::cout << "\nSent broadcast: Client: " << std::string(message.symbol.data()) << ", Net Position: " << message.net_position << ", Timestamp of client: " << std::string(message.timestamp.data()) << std::endl;
                            }
                        });
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        }
    }
}