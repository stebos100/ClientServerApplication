#include "PositionClient.h"
#include "../../include/Common.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <future>

std::mutex print_mutex;
std::random_device rd;
std::mt19937 gen(rd());

void run_Client(const std::string& host, short port, const std::string& symbol_prefix, int index, bool requiresDebugLogs) {

    PositionClient client(host, port, symbol_prefix, requiresDebugLogs);
    // client.start();

    std::uniform_real_distribution<> dis(70.0, 100.0);

    for (int i = 0; i < 10; ++i) {

        if (client.running_) {
            message_t message = {};
            std::string symbol = symbol_prefix;
            std::copy(symbol.begin(), symbol.end(), message.symbol.begin());
            message.net_position = dis(gen);
            client.send_position(message);

            std::this_thread::sleep_for(std::chrono::milliseconds(index));
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    client.disconnect(); 

    std::this_thread::sleep_for(std::chrono::seconds(10));

    client.reconnect();

    for (int i = 0; i < 10; ++i) {

        if (client.running_) {
            message_t message = {};
            std::string symbol = symbol_prefix;
            std::copy(symbol.begin(), symbol.end(), message.symbol.begin());
            message.net_position = dis(gen);
            client.send_position(message);

            std::this_thread::sleep_for(std::chrono::milliseconds(index));
        }
    }

    client.disconnect(); 

    std::this_thread::sleep_for(std::chrono::seconds(5));

    client.stop();
}

int main(int argc, char* argv[]) {

    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " <host> <port> <symbol_prefix> <interimDataSending> <debugLogsRequired>" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    short port = static_cast<short>(std::stoi(argv[2]));
    std::string symbol_prefix = argv[3];
    int interimTimeBetweenMessages = static_cast<short>(std::stoi(argv[4]));
    
    std::string boolString = argv[1];

    bool debugLogs; 

    if(boolString == "true") {
        debugLogs = true;
    }
    else {
        debugLogs = false;
    }

    std::thread client_thread(run_Client, host, port, symbol_prefix, interimTimeBetweenMessages, debugLogs);
    
    client_thread.join();

    return 0;
}