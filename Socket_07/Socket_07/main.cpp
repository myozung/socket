#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>

void recvHandler(int clientSocket) {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    while (true) {
        std::memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytesReceived = read(clientSocket, buffer, BUFFER_SIZE - 1);

        if (bytesReceived <= 0) {
            break;
        }

        std::cout << "> received: " << buffer << std::endl;

        if (std::string(buffer) == "quit") {
            break;
        }
    }
}

int main() {
    const char* HOST = "127.0.0.1";
    const int PORT = 65456;

    std::cout << "> echo-client is activated" << std::endl;

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "socket creation failed" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(HOST);
    serverAddr.sin_port = htons(PORT);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "> connect() failed and program terminated" << std::endl;
        close(clientSocket);
        return 1;
    }

    // recvHandler를 스레드로 별도 실행
    std::thread t([clientSocket]() {
        recvHandler(clientSocket);
    });
    t.detach();

    while (true) {
        std::string sendMsg;
        std::cout << "> ";
        std::getline(std::cin, sendMsg);

        ssize_t bytesSent = write(clientSocket, sendMsg.c_str(), sendMsg.size());
        if (bytesSent == -1) {
            std::cerr << "send failed" << std::endl;
            break;
        }

        if (sendMsg == "quit") {
            break;
        }
    }

    close(clientSocket);
    std::cout << "> echo-client is de-activated" << std::endl;

    return 0;
}

