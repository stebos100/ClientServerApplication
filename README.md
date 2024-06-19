# ClientServerApplication

This project implements a simple server-client application using Boost.Asio for asynchronous network communication. The server can accept multiple clients, and each client sends data to the server. The server broadcasts received data to all clients, and clients print the received data.

Prerequisites
C++17 or later
Boost Libraries (specifically boost_system, boost_thread, and boost_asio)
Installing Boost
Linux (Ubuntu/Debian)
Update the package list:

sh
Copy code
sudo apt update
Install Boost libraries:

sh
Copy code
sudo apt install libboost-all-dev
macOS
Install Homebrew (if not already installed):

sh
Copy code
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
Install Boost using Homebrew:

sh
Copy code
brew install boost
Windows
Download Boost:

Visit the Boost download page and download the latest version of Boost.
Extract Boost:

Extract the downloaded Boost archive to a directory (e.g., C:\local\boost_1_76_0).
Build Boost (optional, if you need specific libraries):

Open a Command Prompt and navigate to the Boost directory.
Run the following commands:
sh
Copy code
bootstrap.bat
.\b2
Compiling the Project
Linux and macOS
Clone the repository:

sh
Copy code
git clone <repository-url>
cd <repository-directory>
Compile the Server and Client:

sh
Copy code
g++ -std=c++17 -I /usr/local/include -L /usr/local/lib -lboost_system -lboost_thread -lpthread PositionServer.cpp mainServer.cpp -o positionServer
g++ -std=c++17 -I /usr/local/include -L /usr/local/lib -lboost_system -lboost_thread -lpthread PositionClient.cpp mainClient.cpp -o positionClient
Windows
Clone the repository:

sh
Copy code
git clone <repository-url>
cd <repository-directory>
Compile the Server and Client:

Open a Command Prompt and navigate to the project directory.
Run the following commands:
sh
Copy code
g++ -std=c++17 -I C:\local\boost_1_76_0 -L C:\local\boost_1_76_0\stage\lib -lboost_system -lboost_thread -lws2_32 PositionServer.cpp mainServer.cpp -o positionServer.exe
g++ -std=c++17 -I C:\local\boost_1_76_0 -L C:\local\boost_1_76_0\stage\lib -lboost_system -lboost_thread -lws2_32 PositionClient.cpp mainClient.cpp -o positionClient.exe
Note: The -lws2_32 linker option is required on Windows for networking.

Running the Application
1. Run the Server
Open a terminal (or Command Prompt on Windows) and start the server:

sh
Copy code
./positionServer # Linux/macOS
positionServer.exe # Windows
2. Run the Clients
Open two more terminals (or Command Prompts on Windows) and start the clients:

Client 1:

sh
Copy code
./positionClient 127.0.0.1 12345 Client1 BTCUSDT.BN # Linux/macOS
positionClient.exe 127.0.0.1 12345 Client1 BTCUSDT.BN # Windows
Client 2:

sh
Copy code
./positionClient 127.0.0.1 12345 Client2 BTCUSDT.HB # Linux/macOS
positionClient.exe 127.0.0.1 12345 Client2 BTCUSDT.HB # Windows
Note
The clients will send their ID to the server upon connection.
The server will reject connections if a client with the same ID already exists.
The clients send random position data to the server, which the server broadcasts to all clients.
Project Files
PositionServer.h and PositionServer.cpp: Server implementation.
PositionClient.h and PositionClient.cpp: Client implementation.
Common.h and Common.cpp: Common definitions and global variables.
mainServer.cpp: Main file to start the server.
mainClient.cpp: Main file to start a client.
Example Output
Server
vbnet
Copy code
Starting PositionServer on port 12345
Accepted connection from: 127.0.0.1 with Client ID: Client1
Accepted connection from: 127.0.0.1 with Client ID: Client2
Sent position to client: Client1, Net Position: 50.5
Sent position to client: Client2, Net Position: 75.3
Client
yaml
Copy code
Received broadcast on ClientID: Client1, Net Position: 50.5, Timestamp of update: 2024-06-18 12:00:00
Received broadcast on ClientID: Client2, Net Position: 75.3, Timestamp of update: 2024-06-18 12:00:00
License
This project is licensed under the MIT License - see the LICENSE file for details.
