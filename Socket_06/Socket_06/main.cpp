#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <algorithm>

std::atomic<bool> serverRunning(true);
std::atomic<int> activeThreads(0);

std::mutex clientListMutex;
std::vector<int> clientSockets;

void handleClient(int clientSocket, sockaddr_in* clientAddr) {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    
    std::cout << "> client connected by IP address "
    << inet_ntoa(clientAddr->sin_addr)
    << " with Port number "
    << ntohs(clientAddr->sin_port) << std::endl;
    
    while (true) {
        std::memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytesReceived = read(clientSocket, buffer, BUFFER_SIZE - 1);
        
        if (bytesReceived <= 0) {
            break;
        }
        
        std::cout << "> echoed: " << buffer
        << " by thread " << std::this_thread::get_id()
        << std::endl;
        
        write(clientSocket, buffer, bytesReceived);
        
        if (strncmp(buffer, "quit", 4) == 0) break;
    }
    
    close(clientSocket);
    delete clientAddr;
    
    {
        std::lock_guard<std::mutex> lock(clientListMutex);
        auto it = std::find(clientSockets.begin(), clientSockets.end(), clientSocket);
        if (it != clientSockets.end()) {
            clientSockets.erase(it);
        }
    }
    
    activeThreads--;
    std::cout << "> active threads are remained : "
    << activeThreads.load() << " threads" << std::endl;
}

void inputThread(int serverSocket) {

    while (true) {
        std::string cmd;
        std::getline(std::cin, cmd);

        if (cmd == "quit") {
            
            std::lock_guard<std::mutex> lock(clientListMutex);
            
            if (!clientSockets.empty()) {

                int s = clientSockets.back();
                shutdown(s, SHUT_RDWR);

                std::cout << "> one client thread terminated" << std::endl;
            }
            else {
                std::cout << "> no active client threads" << std::endl;
            }
      
        }
    }
}


int main() {
    const char* HOST = "127.0.0.1";
    const int PORT = 65456;

    std::cout << "> echo-server is activated" << std::endl;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "socket creation failed" << std::endl;
        return 1;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(HOST);
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "bind failed" << std::endl;
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "listen failed" << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "> Listening on " << HOST << ":" << PORT << std::endl;
    
    std::thread inputMonitor(inputThread, serverSocket);
    inputMonitor.detach();

    while (serverRunning) {
        sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);

            if (!serverRunning) break;
            if (clientSocket == -1) continue;

            activeThreads++;
        
            {
                std::lock_guard<std::mutex> lock(clientListMutex);
                clientSockets.push_back(clientSocket);
            }

            sockaddr_in* clientAddrCopy = new sockaddr_in(clientAddr);

            std::thread t([clientSocket, clientAddrCopy]() {
                handleClient(clientSocket, clientAddrCopy);
            });
            t.detach();
    }
    
    while (activeThreads > 0) {
        usleep(100000);
    }

    close(serverSocket);
    std::cout << "> echo-server is de-activated" << std::endl;

    return 0;
}

