#include <iostream>
#include <cstring>
#include <vector>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <mutex>
#include <algorithm>

std::vector<int> clientList;
std::mutex listMutex;

bool serverRunning = true;
std::mutex runMutex;

int activeThreads = 0;
std::mutex threadMutex;

void broadcastMessage(const char* msg, int length, int senderSocket) {
    std::lock_guard<std::mutex> lock(listMutex);
    
    int count = 0;
    for (int s : clientList)
        if (s != senderSocket)
            count++;

    std::cout << "> received ( " << msg << " ) and echoed to "
              << count << " clients" << std::endl;

    for (int s : clientList) {
        if (s != senderSocket) {
            write(s, msg, length);
        }
    }
}

void handleClient(int clientSocket, sockaddr_in clientAddr) {
    {
           std::lock_guard<std::mutex> lock(threadMutex);
           activeThreads++;
       }
    
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    std::cout << "> client connected by IP address "
              << inet_ntoa(clientAddr.sin_addr)
              << " with Port number "
              << ntohs(clientAddr.sin_port) << std::endl;

    while (true) {
        std::memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytesReceived = read(clientSocket, buffer, BUFFER_SIZE - 1);

        if (bytesReceived <= 0) {
            break;
        }


        if (std::string(buffer) == "quit") {
            break;
        }

        broadcastMessage(buffer, bytesReceived, clientSocket);
    }

    {
        std::lock_guard<std::mutex> lock(listMutex);
        clientList.erase(std::remove(clientList.begin(), clientList.end(), clientSocket), clientList.end());
    }

    close(clientSocket);
    {
            std::lock_guard<std::mutex> lock(threadMutex);
            activeThreads--;
            std::cout << "> active threads are remained : " << activeThreads << " threads" << std::endl;
        }
}

void serverQuitWatcher() {
    while (true) {
        std::string msg;
        std::getline(std::cin, msg);
        
        if (msg == "quit") {
                    std::lock_guard<std::mutex> lock(threadMutex);

                    if (activeThreads == 0) {
                        std::cout << "> stop procedure started" << std::endl;
                        std::lock_guard<std::mutex> r(runMutex);
                        serverRunning = false;
                        break;
                    } else {
                        std::cout << "> active threads are remained : "
                                  << activeThreads << " threads" << std::endl;
                    }
                }
    }
}

int main() {
    const char* HOST = "127.0.0.1";
    const int PORT = 65456;

    std::cout << "> echo-chat-server is activated" << std::endl;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "socket creation failed" << std::endl;
        return 1;
    }

    int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in serverAddr;
        std::memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(HOST);
        serverAddr.sin_port = htons(PORT);

        bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
        listen(serverSocket, 5);

        std::cout << "> Listening on " << HOST << ":" << PORT << std::endl;

        std::thread watcher(serverQuitWatcher);
        watcher.detach();

    while (true) {
        {
            std::lock_guard<std::mutex> lock(runMutex);
            if (!serverRunning)
                break;
        }

        sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == -1) {
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(listMutex);
            clientList.push_back(clientSocket);
        }

        std::thread t(handleClient, clientSocket, clientAddr);
        t.detach();
    }

    close(serverSocket);
    std::cout << "> echo-server is de-activated" << std::endl;

    return 0;
}

