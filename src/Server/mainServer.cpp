#include "PositionServer.h"
#include "../Client/PositionClient.h"
#include "../../include/Common.h"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <random>

std::mutex print_mutex;

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <DebugLogsRequired>" << std::endl;
        return 1;
    }

    std::string boolString = argv[1];

    bool debugLogs; 

    if(boolString == "true") {
        debugLogs = true;
    }
    else {
        debugLogs = false;
    }

    short port = 12345;

    auto server = std::make_shared<PositionServer>(port, debugLogs);

    server->start();

    std::cout << "As an example, I am going to keep the server running for 60 seconds (self set)\n This can be altered for testing OR the server can be closed prematurely by pushing CTRL C..." << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(45));

    // server->stop();

    std::cout << "Server stopped." << std::endl;

    return 0;
}
