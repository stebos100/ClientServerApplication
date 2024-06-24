#ifndef POSITION_CLIENT_H
#define POSITION_CLIENT_H

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/scoped_ptr.hpp>
#include <thread>
#include <atomic>
#include <ctime> 
#include <chrono> 
#include <sstream>
#include <iomanip>
#include "../../include/Message.h"

using boost::asio::ip::tcp;

class PositionClient {
public:
    std::atomic<bool> running_;
    PositionClient(const std::string& host, short port, const std::string& ID, bool& debugLogs, short local_port);
    void start();
    void stop();
    void send_position(message_t& message);
    void request_positions();
    void handle_disconnection();
    void handle_reconnect();
    void disconnect(); 
    void reconnect(); 
    ~PositionClient();

private:
    void do_receive();
    void restart_io_context();    
    bool connect(); 
    bool setConnection();
    void runThreads();
    void process_data(const message_t* message, std::size_t length);

    message_t message_;
    std::string host_;
    short port_;
    short local_port_;
    std::shared_ptr<boost::asio::io_context> io_context_;
    std::optional<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> work_guard_;
    std::unique_ptr<boost::asio::ip::tcp::socket> socket_;
    std::unique_ptr<std::thread> receive_thread_;
    std::vector<char> buffer_;
    std::mutex mutex_; 
    std::string clientID_; 
    bool clientDebugLogs_;
    std::atomic<bool> disconnected_;
    std::atomic<int> reconnectCount;
};

#endif 
