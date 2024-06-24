# ClientServerApplication

This project implements a simple server-client application using Boost.Asio for asynchronous network communication. The server can accept multiple clients, and each client sends data to the server. The server broadcasts received data to all clients, and clients print the received data.

Prerequisites
C++17 or later


Boost Libraries (specifically boost_system, boost_thread, and boost_asio)


# Installing Boost
**Install Boost libraries:**

### Linux (Ubuntu/Debian)

1. **Update the package list:**
```
sudo apt update
```

2. **Install Boost libraries:**

```
sudo apt install libboost-all-dev
```

### macOS
1. **Install Homebrew (if not already installed):**

```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

2. **Install Boost librarues using Homebrew:**

```
brew install boost
```

### Windows
1. Download Boost:

Visit the Boost download page and download the latest version of Boost.
Extract Boost:

2. Extract the downloaded Boost archive to a directory (e.g., C:\local\boost_1_76_0).
3. Build Boost (optional, if you need specific libraries):


Open a Command Prompt and navigate to the Boost directory.
Run the following commands:

```
bootstrap.bat
```

```
.\b2
```

Please note that the default installation path will be ***usr/local/***

# Compiling the Project
**Please note that : /usr/local/include & /usr/local/lib in the below needs to be adjusted if you have a specific install path for Boost**
### Linux (Ubuntu/Debian)

***Clone the repository:***
```
git clone <repository-url>
cd <repository-directory>
```
***Compile/Build the Server and Client files:***

1. **For the server application:**

```
g++ -std=c++17 -g src/Server/mainServer.cpp src/Server/PositionServer.cpp -I include -I /usr/local/include -L /usr/local/lib -o build/PositionServer -lboost_system -lboost_thread -lpthread
```

2. **For the Client application:**
```
g++ -std=c++17 -g src/Client/mainClient.cpp src/Client/PositionClient.cpp -I include -I /usr/local/include -L /usr/local/lib -o build/PositionClient -lboost_system -lboost_thread -lpthread
```

### Windows

***Clone the repository:***
```
git clone <repository-url>
cd <repository-directory>
```
***Compile/Build the Server and Client files:***

1. **For the server application:**

```
g++ -std=c++17 -g src\\Server\\mainServer.cpp src\\Server\\PositionServer.cpp -I include -I C:\\local\\boost_1_76_0 -L C:\\local\\boost_1_76_0\\stage\\lib -o build\\PositionServer.exe -lboost_system -lboost_thread -lws2_32
```

2. **For the Client application:**

```
g++ -std=c++17 -g src\\Client\\mainClient.cpp src\\Client\\PositionClient.cpp -I include -I C:\\local\\boost_1_76_0 -L C:\\local\\boost_1_76_0\\stage\\lib -o build\\PositionClient.exe -lboost_system -lboost_thread -lws2_32
```

**Note: The -lws2_32 linker option is required on Windows for networking**

### To run the intedned Application: 

#### 1. Open a Command Prompt and navigate to the project directory.
#### 2. Run the following commands to initialize the Server applicion (Please note that the server will now be running)

##### Run the following commands in shell 

```
./positionServer false # Linux/macOS
positionServer.exe false # Windows
```

**Please note that the client application has two command line arguments**

1. **The executable file (.exe)**
2. **(Boolean) is whether the user would like debug Log files to be printed, for the test application it is set to false, but if you would like a more detailed view of the interaction please set this to true**

**For the Client application:**

#### 3. Open a new Command Prompt and navigate to the project directory.
#### 4. Run the following commands to initialize the Server applicion:

##### Run the following commands in shell (The more teminals openend in this manner the better !! ie more clients) 

```
./positionClient 127.0.0.1 12345 BTCUSDT.BKN 1200 false 1201 0 # Linux/macOS
./positionClient.exe 127.0.0.1 12345 BTCUSDT.BKN 1200 false 1201 0 # Windows
```
**Please note that the above application has 8 fields which need to be fed into the command line**

1. **The executable file (.exe)**
2. **The host address (127....)**
3. **The Server Address that the client needs to access (12345)**
4. **The unique client ID+exchange (BTCUSDT.BN)**
5. **How often the clients position will upate (in milliseconds) (800: for testing I Please limit this to less 2000 or extend server lifetime in mainServer.cpp)**
6. **Boolean value if we would like the debug logs to be printed, default is false (false)**
7. **The local port number to ensure the socket IDs are unique (int)**
8. **If you want the client thread to assume function 1 or 2 in the clientMain.cpp (used for testing)**

#### 5. Repeat steps 3 and 4 in different terminals with different client names, this will ensure maximal interaction between server and client

## Notes
The clients will send their ID to the server upon connection.

The server will reject connections if a client with the same ID already exists.

The clients send random position data to the server, which the server broadcasts to all clients.

## Project Files

PositionServer.h and PositionServer.cpp: Server implementation (Located in src/Server).

PositionClient.h and PositionClient.cpp: Client implementation (Located in src/Client).

Common.h: Common definitions and global variables.

mainServer.cpp: Main file to start the server(Located in src/Server).

mainClient.cpp: Main file to start a client(Located in src/Client).

## Example for Testing (The default): 

#### command lines used for Server: 

##### terminal 1
```
./positionServer false # Linux/macOS
```
#### command lines used for Clients (Each in a differnt terminal) :

##### terminal 2
```
./positionClient 127.0.0.1 12345 BTCUSDT.BKN 1200 false 1201 0 # Linux/macOS
```
##### terminal 3
```
./positionClient 127.0.0.1 12345 BTCUSDT.BN 1200 false 1202 0 # Linux/macOS
```
##### terminal 4
```
./positionClient 127.0.0.1 12345 BTCUSDT.KRKN 1200 false 1203 0 # Linux/macOS
```

### Example Output From server: 

```
PositionServer constructed and acceptor initialized on port 12345
Starting PositionServer on port 12345
Starting worker threads
Running io_context
As an example, I am going to keep the server running for 60 seconds (self set)
This can be altered for testing OR the server can be closed prematurely by pushing CTRL C...

Accepted connection from: 127.0.0.1:64573
Received message from client: BTCUSDT.BKN, net position: 123.45, timestamp: 2024-Jun-24 09:56:13
Accepted connection from: 127.0.0.1:64574
Received message from client: BTCUSDT.BN, net position: 123.45, timestamp: 2024-Jun-24 09:56:14
Sending BroadCast to: 127.0.0.1:64574

Sent broadcast: Client position upon joining to Client BTCUSDT.BN:|   BTCUSDT.BKN, Net Position: 95.9567, Timestamp of client: 2024-Jun-24 09:56:14

Client BTCUSDT.BKN closed connection.
Client 127.0.0.1 disconnected and removed from the set.
Client BTCUSDT.BKN disconnected and removed from the set.

Accepted connection from: 127.0.0.1:64575
Received message from client: BTCUSDT.BKN, net position: 123.45, timestamp: 2024-Jun-24 09:56:28
Sending BroadCast to: 127.0.0.1:64575

Sent broadcast: Client position upon joining to Client BTCUSDT.BKN:|   BTCUSDT.BN, Net Position: 88.8986, Timestamp of client: 2024-Jun-24 09:56:27

Server stopped.
Stopping server...
Server resources cleaned up and stopped.

```
### Example Output From Client (Terminal 2: BTCUSDT.BN):

```
Received broadcast on ClientID: BTCUSDT.BN| Update for Client: BTCUSDT.HB, Net Position: 71.4657, Timestamp of update: 2024-Jun-19 18:05:44

Received broadcast on ClientID: BTCUSDT.BN| Update for Client: BTCUSDT.KRKN, Net Position: 76.9948, Timestamp of update: 2024-Jun-19 18:05:44

Received broadcast on ClientID: BTCUSDT.BN| Update for Client: BTCUSDT.HB, Net Position: 78.4795, Timestamp of update: 2024-Jun-19 18:05:44

Received broadcast on ClientID: BTCUSDT.BN| Update for Client: BTCUSDT.KRKN, Net Position: 99.3953, Timestamp of update: 2024-Jun-19 18:05:45

Received broadcast on ClientID: BTCUSDT.BN| Update for Client: BTCUSDT.KRKN, Net Position: 78.8712, Timestamp of update: 2024-Jun-19 18:05:45

Received broadcast on ClientID: BTCUSDT.BN| Update for Client: BTCUSDT.HB, Net Position: 83.1269, Timestamp of update: 2024-Jun-19 18:05:45

.......

Received broadcast on ClientID: BTCUSDT.BN| Update for Client: BTCUSDT.HB, Net Position: 80.3377, Timestamp of update: 2024-Jun-19 18:05:54

Received broadcast on ClientID: BTCUSDT.BN| Update for Client: BTCUSDT.HB, Net Position: 71.4785, Timestamp of update: 2024-Jun-19 18:05:55

Received broadcast on ClientID: BTCUSDT.BN| Update for Client: BTCUSDT.HB, Net Position: 80.6918, Timestamp of update: 2024-Jun-19 18:05:55

Received broadcast on ClientID: BTCUSDT.BN| Update for Client: BTCUSDT.HB, Net Position: 77.865, Timestamp of update: 2024-Jun-19 18:05:56

Successfully joined client thread...
Position client has been destructed...
```
## Simulating High-Frequency Trading (HFT) Interactions

In high-frequency trading (HFT), multiple trading systems or exchanges interact with each other at extremely high speeds to capitalize on market inefficiencies. To effectively simulate these interactions, it is essential to represent each exchange as an independent entity that can operate concurrently and communicate with a central server. This is why we run multiple clients in different terminals.

### Why Did I use Multiple Terminals for Clients?

1. **Isolation of Processes**:
   Each terminal represents an independent client process, simulating a separate exchange or trading system. This isolation ensures that the behavior of one client does not directly interfere with others, providing a more realistic simulation of a distributed trading environment.

2. **Concurrent Interactions**:
   Running multiple clients concurrently allows us to simulate real-world scenarios where multiple exchanges interact with a central server simultaneously. This concurrency is crucial for testing the performance and robustness of the server under different conditions.

3. **Independent Data Streams**:
   Each client can send and receive its own data stream, representing different market data and orders. This diversity in data helps test the server's ability to handle various types of information and ensures that the system can process and broadcast updates accurately and efficiently.

4. **Realistic Testing Environment**:
   In HFT, exchanges operate independently but interact frequently. By running multiple clients, we can test the server's ability to handle rapid data exchange and ensure that it maintains consistency and accuracy across all connected clients.

### Why a TCP server application ? 

#### Correctness: 

TCP ensures all clients have a consistent view of positions by guaranteeing reliable and error-free data delivery. This keeps all strategies on the same page.

#### Order Preservation: 
TCP delivers data in the exact order it was sent, which is crucial in HFT where the sequence of updates matters. No risk of scrambled data.

#### Resilience: 
TCP handles dropped connections and packet loss with retransmissions and flow control, ensuring no information is lost even if there are network issues.

In short, TCP is the best fit for ensuring our HFT desk's strategies stay accurate and synchronized.



