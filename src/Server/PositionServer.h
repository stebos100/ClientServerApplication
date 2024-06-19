#ifndef POSITION_SERVER_H
#define POSITION_SERVER_H

#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
#include "../../include/Message.h"

using boost::asio::ip::tcp;

class PositionServer : public std::enable_shared_from_this<PositionServer> {
public:
    PositionServer(short port,  bool& debugLogs);
    ~PositionServer();
    void start();
    void stop();

private:
    void do_accept();
    void handle_client(std::shared_ptr<tcp::socket> socket, const std::string& client_id);
    void enqueue_message(const message_t& message);
    void process_messages();
    void handle_position_request(std::shared_ptr<tcp::socket> socket, const std::string& clientID);

    boost::asio::io_context io_context_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
    tcp::acceptor acceptor_;
    std::unordered_set<std::shared_ptr<tcp::socket>> clients_;
    std::unordered_set<std::string> connected_client_ids_;
    std::unordered_map<std::string, message_t> client_positions_;
    std::mutex clients_mutex_;
    std::condition_variable message_condition_;
    boost::lockfree::queue<message_t> message_queue_;
    std::atomic<bool> running_;
    std::vector<std::thread> worker_threads_;
    std::thread accept_thread_;
    bool debugLogs_; 

};

#endif // POSITION_SERVER_H

