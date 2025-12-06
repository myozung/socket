#include <iostream>
#include <cstring>
#include <vector>
#include <mutex>
#include <algorithm>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct ClientInfo {
    sockaddr_in addr;
};

std::vector<ClientInfo> clientList;
std::mutex listMutex;

bool sameClient(const sockaddr_in& a, const sockaddr_in& b) {
    return a.sin_addr.s_addr == b.sin_addr.s_addr &&
           a.sin_port == b.sin_port;
}

int main() {
    const char* HOST = "127.0.0.1";
    const int PORT = 65456;
    const int BUFFER_SIZE = 1024;

    std::cout << "> echo-server is activated" << std::endl;


    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1) {
        std::cerr << "socket creation failed" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(HOST);
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "bind failed" << std::endl;
        close(serverSocket);
        return 1;
    }

    char buffer[BUFFER_SIZE];

    while (true) {
        sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        std::memset(buffer, 0, BUFFER_SIZE);

        ssize_t bytesReceived = recvfrom(
            serverSocket,
            buffer,
            BUFFER_SIZE - 1,
            0,
            (sockaddr*)&clientAddr,
            &addrLen
        );

        if (bytesReceived <= 0)
            continue;

        std::string msg(buffer);

        if (msg == "quit")
            continue;
                

        if (msg == "#REG") {
            std::lock_guard<std::mutex> lock(listMutex);

            bool exists = false;
            for (auto& c : clientList)
                if (sameClient(c.addr, clientAddr))
                    exists = true;

            if (!exists) {
                ClientInfo ci;
                ci.addr = clientAddr;
                clientList.push_back(ci);

                std::cout << "> client registered ('"
                          << inet_ntoa(clientAddr.sin_addr)
                          << "', " << ntohs(clientAddr.sin_port) << ")"
                          << std::endl;
            }

            continue;
        }


        if (msg == "#DEREG") {
            std::lock_guard<std::mutex> lock(listMutex);

            clientList.erase(
                std::remove_if(
                    clientList.begin(),
                    clientList.end(),
                    [&](ClientInfo& c) { return sameClient(c.addr, clientAddr); }
                ),
                clientList.end()
            );

            std::cout << "> client de-registered ('"
                      << inet_ntoa(clientAddr.sin_addr)
                      << "', " << ntohs(clientAddr.sin_port) << ")"
                      << std::endl;

            continue;
        }


        bool isRegistered = false;
        {
            std::lock_guard<std::mutex> lock(listMutex);

            for (auto& c : clientList)
                if (sameClient(c.addr, clientAddr))
                    isRegistered = true;
        }

        if (!isRegistered) {
            std::cout << "> no clients to echo" << std::endl;
            continue;
        }


        int echoCount = 0;
        {
            std::lock_guard<std::mutex> lock(listMutex);
            echoCount = clientList.size();

            std::cout << "> received ( " << msg
                      << " ) and echoed to " << echoCount
                      << " clients" << std::endl;

            for (auto& c : clientList) {
                sendto(
                    serverSocket,
                    buffer,
                    bytesReceived,
                    0,
                    (sockaddr*)&c.addr,
                    sizeof(c.addr)
                );
            }
        }
    }

    close(serverSocket);
    return 0;
}

