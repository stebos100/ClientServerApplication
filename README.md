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
3. 
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

**Please note that the client application has two command line arguments, 1 to execute the .exe file, and the second (Boolean) is whether the user would like debug Log files to be printed, for the test application it is set to false, but if you would like a more detailed view of the interaction please set this to true**


**For the Client application:**

#### 3. Open a new Command Prompt and navigate to the project directory.
#### 4. Run the following commands to initialize the Server applicion:

##### Run the following commands in shell (The more teminals openend in this manner the better !! ie more clients) 

```
./positionClient.exe 127.0.0.1 12345 BTCUSDT.BKN 800 true# Linux/macOS
positionClient.exe 127.0.0.1 12345 BTCUSDT.BKN 800 true # Windows
```
**Please note that the above application has 6 fields which need to be fed into the command line**

1. **The executable file (.exe)**
2. **The host address (127....)**
3. **The Server Address that the client needs to access (12345)**
4. **The unique client ID+exchange (BTCUSDT.BN)**
5. **How often the clients position will upate (in milliseconds) (800)**
6. **If we would like the debug logs to be printed, default is false (false)**

#### 5. Repeat steps 3 and 4 in different terminals with different client names, this will ensure maximal interaction between server and client


## Simulating High-Frequency Trading (HFT) Interactions

In high-frequency trading (HFT), multiple trading systems or exchanges interact with each other at extremely high speeds to capitalize on market inefficiencies. To effectively simulate these interactions, it is essential to represent each exchange as an independent entity that can operate concurrently and communicate with a central server. This is why we run multiple clients in different terminals.

### Why Use Multiple Terminals for Clients?

1. **Isolation of Processes**:
   Each terminal represents an independent client process, simulating a separate exchange or trading system. This isolation ensures that the behavior of one client does not directly interfere with others, providing a more realistic simulation of a distributed trading environment.

2. **Concurrent Interactions**:
   Running multiple clients concurrently allows us to simulate real-world scenarios where multiple exchanges interact with a central server simultaneously. This concurrency is crucial for testing the performance and robustness of the server under conditions that closely mimic live trading environments.

3. **Independent Data Streams**:
   Each client can send and receive its own data stream, representing different market data and orders. This diversity in data helps test the server's ability to handle various types of information and ensures that the system can process and broadcast updates accurately and efficiently.

4. **Realistic Testing Environment**:
   In HFT, exchanges operate independently but interact frequently. By running multiple clients, we can test the server's ability to handle rapid data exchange and ensure that it maintains consistency and accuracy across all connected clients. This setup helps identify potential bottlenecks or issues in the server's performance.

### Example Use Case

Imagine running three clients in three different terminals, each representing a different exchange (e.g., Exchange A, Exchange B, and Exchange C). These clients can send their respective market data and order updates to the central server. The server, in turn, broadcasts these updates to all connected clients, simulating the real-time data flow and interaction between exchanges in an HFT environment.

By running this simulation, we can observe how the server handles multiple data streams, processes information concurrently, and maintains data integrity across all clients. This setup provides valuable insights into the system's performance and reliability, which are critical factors in high-frequency trading.

Running multiple clients in different terminals helps us create a realistic and effective testing environment for our server-client application, ensuring that it can meet the demands of high-frequency trading.

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

### Example for Testing (The default): 

#### command lines used for Server: 

##### terminal 1
```
./positionServer false # Linux/macOS
```
#### command lines used for Clients (Each in a differnt terminal) :

##### terminal 2
```
./positionClient.exe 127.0.0.1 12345 BTCUSDT.BN 800 false
```
##### terminal 3
```
./positionClient.exe 127.0.0.1 12345 BTCUSDT.HB 500 false
```
##### terminal 4
```
./positionClient.exe 127.0.0.1 12345 BTCUSDT.KRKN 900 false
```

### Example Output From server: 



