#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    const char* HOST = "127.0.0.1";
    const int PORT = 65456;
    const int BUFFER_SIZE = 1024;

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
        std::cerr << "connect failed" << std::endl;
        close(clientSocket);
        return 1;
    }

    char buffer[BUFFER_SIZE];

    while (true) {
        std::cout << "> ";
        std::string sendMsg;
        std::getline(std::cin, sendMsg);

        ssize_t bytesSent = write(clientSocket, sendMsg.c_str(), sendMsg.size());
        if (bytesSent == -1) {
            std::cerr << "send failed" << std::endl;
            break;
        }

        std::memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytesReceived = read(clientSocket, buffer, BUFFER_SIZE - 1);
        if (bytesReceived <= 0) {
            std::cerr << "server disconnected" << std::endl;
            break;
        }

        std::cout << "> received: " << buffer << std::endl;

        if (sendMsg == "quit") {
            break;
        }
    }

    close(clientSocket);
    std::cout << "> echo-client is de-activated" << std::endl;

    return 0;
}
myozung@hyojeong-2 풀서네 % cd socket
myozung@hyojeong-2 socket % ls -a
.        .DS_Store    Socket_01
..        .git        Socket_02
myozung@hyojeong-2 socket % cd Socket_02
myozung@hyojeong-2 Socket_02 % ls -a
.        ..        .DS_Store    Socket_02
myozung@hyojeong-2 Socket_02 % cd Socket_02
myozung@hyojeong-2 Socket_02 % ls -a
.            .DS_Store        Socket_02.xcodeproj
..            Socket_02
myozung@hyojeong-2 Socket_02 % cd Socket_02
myozung@hyojeong-2 Socket_02 % ls -a
.        ..        main.cpp
myozung@hyojeong-2 Socket_02 % git add main.cpp
fatal: 내용이 없는 하위 모듈 'Socket_02'에서
