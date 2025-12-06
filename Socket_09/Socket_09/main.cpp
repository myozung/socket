#include <iostream>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

bool running = true;

void recvHandler(int clientSocket) {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    sockaddr_in serverAddr;
    socklen_t addrLen = sizeof(serverAddr);

    while (running) {
        std::memset(buffer, 0, BUFFER_SIZE);

        ssize_t bytesReceived = recvfrom(
            clientSocket,
            buffer,
            BUFFER_SIZE - 1,
            0,
            (sockaddr*)&serverAddr,
            &addrLen
        );

        if (bytesReceived > 0) {
            std::cout << "> received: " << buffer << std::endl;

            if (std::string(buffer) == "quit") {
                break;
            }
        }
    }
}

int main() {
    const char* HOST = "127.0.0.1";
    const int PORT = 65456;  // UDP echo server 포트
    const int BUFFER_SIZE = 1024;

    std::cout << "> udp-echo-client is activated" << std::endl;

    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == -1) {
        std::cerr << "socket creation failed" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(HOST);
    serverAddr.sin_port = htons(PORT);

    // 수신 스레드 시작
    std::thread t(recvHandler, clientSocket);
    t.detach();

    while (true) {
        std::string msg;
        std::cout << "> ";
        std::getline(std::cin, msg);

        ssize_t bytesSent = sendto(
            clientSocket,
            msg.c_str(),
            msg.size(),
            0,
            (sockaddr*)&serverAddr,
            sizeof(serverAddr)
        );

        if (bytesSent == -1) {
            std::cerr << "send failed" << std::endl;
            break;
        }

        if (msg == "quit") {
            running = false;
            break;
        }
    }

    close(clientSocket);
    std::cout << "> udp-echo-client is de-activated" << std::endl;

    return 0;
}

