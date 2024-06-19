#ifndef POSITION_CLIENT_H
#define POSITION_CLIENT_H

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
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
    PositionClient(const std::string& host, short port, const std::string& ID, bool& debugLogs);
    void start();
    void stop();
    void send_position(const message_t& message);
    void request_positions();
    void handle_disconnection();
    ~PositionClient();

private:
    void do_receive();

    boost::asio::io_context io_context_;
    tcp::socket socket_;
    std::thread receive_thread_;
    std::atomic<bool> running_;
    std::mutex mutex_; 
    std::string clientID_; 
    bool clientDebugLogs_;
};

#endif // POSITION_CLIENT_H
