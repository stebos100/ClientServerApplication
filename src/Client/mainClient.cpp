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

void run_Client(const std::string& host, short port, const std::string& symbol_prefix, int index, bool requiresDebugLogs, short lclPort) {

    PositionClient client(host, port, symbol_prefix, requiresDebugLogs, lclPort);
    // client.start();

    std::uniform_real_distribution<> dis(70.0, 100.0);

    for (int i = 0; i < 10; ++i) {

        if (client.running_) {
            message_t message = {};
            std::string symbol = symbol_prefix;
            std::copy(symbol.begin(), symbol.end(), message.symbol.begin());
            message.net_position = dis(gen);
            client.send_position(message);

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    client.reconnect();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    for (int i = 0; i < 40; ++i) {

        message_t message = {};
        std::string symbol = symbol_prefix;
        std::copy(symbol.begin(), symbol.end(), message.symbol.begin());
        message.net_position = dis(gen);
        client.send_position(message);

        std::this_thread::sleep_for(std::chrono::milliseconds(index));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(index));

    client.stop();
}

void run_ClientTwo(const std::string& host, short port, const std::string& symbol_prefix, int index, bool requiresDebugLogs, short lclPort) {

    PositionClient client(host, port, symbol_prefix, requiresDebugLogs, lclPort);

    std::uniform_real_distribution<> dis(70.0, 100.0);

    for (int i = 0; i < 30; ++i) {

        if (client.running_) {
            message_t message = {};
            std::string symbol = symbol_prefix;
            std::copy(symbol.begin(), symbol.end(), message.symbol.begin());
            message.net_position = dis(gen);
            client.send_position(message);

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
}

int main(int argc, char* argv[]) {

    if (argc != 8) {
        std::cerr << "Usage: " << argv[0] << " <host> <port> <symbol_prefix> <interimDataSending> <debugLogsRequired> <localPortNumber> <clientFunctionSpecifier>" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    short port = static_cast<short>(std::stoi(argv[2]));
    std::string symbol_prefix = argv[3];
    int interimTimeBetweenMessages = static_cast<short>(std::stoi(argv[4]));
    
    std::string boolString = argv[5];
    short local_port = static_cast<short>(std::stoi(argv[6]));
    int spec = static_cast<int>(std::stoi(argv[7]));

    bool debugLogs; 

    if(boolString == "true") {
        debugLogs = true;
    }
    else {
        debugLogs = false;
    }

    if(spec == 0) {
        std::thread client_thread(run_Client, host, port, symbol_prefix, interimTimeBetweenMessages, debugLogs, local_port);

        client_thread.join();
    }
    else  if (spec == 1) {

        std::thread client_thread(run_ClientTwo, host, port, symbol_prefix, interimTimeBetweenMessages, debugLogs, local_port);
        
        client_thread.join();
    }



    return 0;
}