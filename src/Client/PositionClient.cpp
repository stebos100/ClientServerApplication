#include "PositionClient.h"
#include <iostream>
#include "../../include/Common.h"

PositionClient::PositionClient(const std::string& host, short port, const std::string& clientID, bool& debugLogs, short local_port)
    :   io_context_(std::make_shared<boost::asio::io_context>()), host_(host), port_(port), socket_(std::make_unique<tcp::socket>(*io_context_)), running_(false), local_port_(local_port),
      clientID_(clientID), clientDebugLogs_(debugLogs), buffer_(sizeof(message_t)), 
      work_guard_(boost::asio::make_work_guard(*io_context_)) {

    reconnectCount = 0;

    std::cout << "Starting client...\n";

    connect();

    receive_thread_ = std::make_unique<std::thread>([this]() {
        try {
            io_context_->run();
        } catch (const std::exception& e) {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cerr << "Exception in io_context.run(): " << e.what() << std::endl;
        }
    });

}

PositionClient::~PositionClient() {

    stop();
    std::lock_guard<std::mutex> lock(print_mutex);
    std::cout << "Position client has been destructed...\n"; 
}


void PositionClient::start() {

    std::cout << "Starting client...\n";

    work_guard_.reset(); 
    work_guard_.emplace(boost::asio::make_work_guard(*io_context_));

    connect();

    receive_thread_.reset();

    receive_thread_ = std::make_unique<std::thread>([this]() {
        try {
            io_context_->run();
        } catch (const std::exception& e) {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cerr << "Exception in io_context.run(): " << e.what() << std::endl;
        }
    });

}

void PositionClient::stop() {

    // if (!running_) return;

    running_ = false;
    disconnected_ = true;

    boost::system::error_code ignore;

    if (socket_ && socket_->is_open()) {
        socket_->close();
    }

    io_context_->stop();

    if (receive_thread_ && receive_thread_->joinable()) {
        receive_thread_->join();
    }

    std::lock_guard<std::mutex> lock(print_mutex);
    std::cout << "Socket and io_context stopped, thread joined.\n";

    io_context_->reset();
}

bool PositionClient::connect() {

    std::cout << "We have entered the connect function...\n";

    try {

        {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << "Attempting to connect...\n";
        }

        socket_ = std::make_unique<tcp::socket>(*io_context_);

        tcp::resolver resolver(*io_context_);
        auto endpoints = resolver.resolve(host_, std::to_string(port_));

        tcp::endpoint local_endpoint(tcp::v4(), local_port_);
        socket_->open(tcp::v4());
        socket_->bind(local_endpoint);

        boost::system::error_code ec;
        boost::asio::connect(*socket_, endpoints, ec);

        if (ec) {

            {
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cerr << "Connection failed: " << ec.message() << std::endl;
            }

            socket_->close();
            return false;
        }

        {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << "Connected to server.\n";
            std::cout << "NOW OPERATING ON :" << socket_->remote_endpoint().address().to_string() << ":" << socket_->remote_endpoint().port() << std::endl;
            running_ = true;
        }

        message_t message;
        std::strncpy(message.symbol.data(), clientID_.data(), message.symbol.size());
        message.net_position = 123.45;

        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        std::string timestamp_str = boost::posix_time::to_simple_string(now);
        std::copy(timestamp_str.begin(), timestamp_str.end(), message.timestamp.begin());

        std::memcpy(buffer_.data(), &message, sizeof(message));

        boost::system::error_code error;
        socket_->write_some(boost::asio::buffer(buffer_, sizeof(message_t)), error);

        if (error) {

            {
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cerr << "Failed to send client ID: " << error.message() << std::endl;
            }

            socket_->close();
            running_ = false;
            return false;
        }

        do_receive(); 
        return true;

    } catch (const std::exception& e) {

        std::lock_guard<std::mutex> lock(print_mutex);
        std::cerr << "Failed to connect: " << e.what() << std::endl;
        return false;
    }
}

bool PositionClient::setConnection() {

    std::cout << "We have entered the connect function...\n";

    try {

        {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << "Attempting to connect...\n";
        }

        socket_ = std::make_unique<tcp::socket>(*io_context_);

        tcp::resolver resolver(*io_context_);
        auto endpoints = resolver.resolve(host_, std::to_string(port_));

        if (local_port_ != 0) {

            tcp::endpoint local_endpoint(tcp::v4(), local_port_);
            socket_->open(tcp::v4());
            socket_->bind(local_endpoint);
        }

        boost::system::error_code ec;
        boost::asio::connect(*socket_, endpoints, ec);

        if (ec) {

            {
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cerr << "Connection failed: " << ec.message() << std::endl;
            }

            socket_->close();
            return false;
        }

        {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << "Connected to server.\n";
            running_ = true;
        }

        message_t message;
        std::strncpy(message.symbol.data(), clientID_.data(), message.symbol.size());
        message.net_position = 123.45;

        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        std::string timestamp_str = boost::posix_time::to_simple_string(now);
        std::copy(timestamp_str.begin(), timestamp_str.end(), message.timestamp.begin());

        std::memcpy(buffer_.data(), &message, sizeof(message));

        boost::system::error_code error;
        socket_->write_some(boost::asio::buffer(buffer_, sizeof(message_t)), error);

        if (error) {

            {
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cerr << "Failed to send client ID: " << error.message() << std::endl;
            }

            socket_->close();
            running_ = false;
            return false;
        }

        return true;

    } catch (const std::exception& e) {

        std::lock_guard<std::mutex> lock(print_mutex);
        std::cerr << "Failed to connect: " << e.what() << std::endl;
        return false;
    }
}

void PositionClient::restart_io_context() {

    io_context_->reset();
    work_guard_.reset(); 
    work_guard_.emplace(boost::asio::make_work_guard(*io_context_));

    if (receive_thread_ && receive_thread_->joinable()) {

        receive_thread_->join();

        {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << "recieve thread has been joined..." << std::endl;
        }
    }

    receive_thread_ = std::make_unique<std::thread>([this]() {
        try {
            io_context_->run();
        } catch (const std::exception& e) {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cerr << "Exception in io_context.run(): " << e.what() << std::endl;
        }
    });
}

void PositionClient::runThreads() {

    if (receive_thread_ && receive_thread_->joinable() && io_context_ -> stopped()) {

        {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << "attmepting to join running thread.." << std::endl;
        }

        receive_thread_->join();

        {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << "Running thread has been joined..." << std::endl;
        }
    }

     receive_thread_ = std::make_unique<std::thread>([this]() {
        try {
            io_context_->run();
        } catch (const std::exception& e) {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cerr << "Exception in io_context.run(): " << e.what() << std::endl;
        }
    });
}

void PositionClient::handle_disconnection() {

    if(!running_ && disconnected_)
    {
        {
            std::lock_guard<std::mutex> lock(print_mutex); 
            std::cout << "DISCONNECTION HAS ALREADY BEEN INTERNALLY INITIATED\n";
        }   

        return; 
    }

    running_ = false;

    {
        std::lock_guard<std::mutex> lock(print_mutex); 
        std::cout << "DISCONNECTION CALLED INTERNALLY FROM FUNCTION FAILURE....\nAttempting to disconnect...\n";
    }

    if (socket_ && socket_->is_open()) {

        boost::system::error_code ec;

        socket_->close(ec);
        socket_.reset();

        if (!ec) {

            {
                std::lock_guard<std::mutex> lock(print_mutex); 
                std::cout <<"DISCONNECTION SUCCESSFUL....\n";
            }

            disconnected_ = true; 
        
            handle_reconnect();

        } else {

            {
                std::lock_guard<std::mutex> lock(print_mutex); 
                std::cerr << "Disconnection failed: " << ec.message() << "\n";
                handle_disconnection();
            }
        }
    }
}

void PositionClient::handle_reconnect() {

    if (disconnected_ && !running_) {

        {
            std::lock_guard<std::mutex> lock(print_mutex); 
            std::cout <<"RECONNECTION CALLED INTERNALLY FROM FUNCTION FAILURE....\n";
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    restart_io_context();

    if (connect()) {
            std::lock_guard<std::mutex> lock(print_mutex); 
            std::cout <<"RECONNECTION SUCCESSFUL....\n";
            std::cout << "NOW OPERATING ON :" << socket_->remote_endpoint().address().to_string() << ":" << socket_->remote_endpoint().port() << std::endl;
            reconnectCount = 0;
            return;
    } else {

        {
            std::lock_guard<std::mutex> lock(print_mutex); 
            std::cout <<"RECONNECTION UNSUCCESSFUL....\n";
            reconnectCount++; 
        }

        if (reconnectCount < 3) {
            handle_reconnect();
        }
        else {
            stop();
        }
    }
}

void PositionClient::disconnect() {

    if(disconnected_) {
        return; 
    }

    running_ = false;

    {
        std::lock_guard<std::mutex> lock(print_mutex); 
        std::cout <<"DISCONNECTION CALLED BY CLIENT....\n";
    }
    
    if (socket_ && socket_->is_open()) {

        boost::system::error_code ec;
        socket_->close(ec);
        socket_.reset();

        if (!ec) {

            {
                std::lock_guard<std::mutex> lock(print_mutex); 
                std::cout <<"DISCONNECTION SUCCESSFUL....\n";
            }

            disconnected_ = true; 
            return;

        } else {

            {
                std::lock_guard<std::mutex> lock(print_mutex); 
                std::cerr << "Disconnection failed: " << ec.message() << "\n";
            }

            disconnect();
        }
    } else {

        std::lock_guard<std::mutex> lock(print_mutex); 
        std::cout <<"SOCKET ALREADY CLOSED AND SHUTDOWN....\n";
    }
}

void PositionClient::reconnect() {

    disconnect();

    std::this_thread::sleep_for(std::chrono::seconds(5));

    {
        std::lock_guard<std::mutex> lock(print_mutex); 
        std::cout <<"RECONNECTION CALLED MANUALLY FROM CLIENT....\n";
    }

    restart_io_context();

    if (connect()) {
        std::lock_guard<std::mutex> lock(print_mutex); 
        std::cout <<"RECONNECTION SUCCESSFUL....\n";
        std::cout << "NOW OPERATING ON :" << socket_->remote_endpoint().address().to_string() << ":" << socket_->remote_endpoint().port() << std::endl;
        reconnectCount = 0;
        return;
    } else {

        {
            std::lock_guard<std::mutex> lock(print_mutex); 
            std::cout <<"RECONNECTION UNSUCCESSFUL....\n";
        }

        if (reconnectCount < 3) {
            reconnect();
        }
        else {
            stop();
        }
    }
}

void PositionClient::send_position(message_t& message) {

    if (!socket_->is_open() && running_) {

        {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cerr << "Socket is not open. Cannot send message.\n";
        }

        return;
    }

    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    std::string timestamp_str = boost::posix_time::to_simple_string(now);
    std::copy(timestamp_str.begin(), timestamp_str.end(), message.timestamp.begin());

    // std::memcpy(buffer_.data(), &message, sizeof(message));

    boost::asio::async_write(*socket_, boost::asio::buffer(&message, sizeof(message)),
        [this](boost::system::error_code ec, std::size_t /*length*/) {
            if (ec) {
                std::lock_guard<std::mutex> lock(print_mutex);
                std::cerr << "Failed to send message: " << ec.message() << std::endl;
            }
        });
}

void PositionClient::do_receive() {

    if (!socket_->is_open() || !running_) {
        std::cout << "Error in do_receive: socket not open or client not running.\n";
        return;
    }

    // std::cout << "We have made it back to the receive function..\n";

    boost::asio::async_read(*socket_, boost::asio::buffer(&message_, sizeof(message_t)),
        [this](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                // std::cout << "No errors here. Processing data...\n";
                process_data(&message_, length);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                do_receive(); 
            } else {

                {
                    std::lock_guard<std::mutex> lock(print_mutex);
                    std::cerr << "Read error: " << ec.message() << std::endl;
                }

                if (running_) {

                    std::cout << "Handling disconnection...\n";
                    handle_disconnection();
                }

                return;
            }
        });
}

void PositionClient::process_data(const message_t* message, std::size_t length) {

    std::lock_guard<std::mutex> lock(print_mutex);
    std::string symbol(message->symbol.data());

    // std::cout << "we have entered the process data function\n";

    if (symbol != clientID_) {

        std::cout << "\nReceived broadcast on ClientID: " << clientID_ << "| Update for Client: "
                  << symbol << ", Net Position: " << message->net_position
                  << ", Timestamp of update: " << std::string(message->timestamp.data()) << std::endl;
    }
}
