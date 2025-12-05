#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    const char* HOST = "127.0.0.1";
    const int PORT = 65456;
    const int BUFFER_SIZE = 1024;

    std::cout << "> udp-echo-server is activated" << std::endl;

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

    std::cout << "> Listening on " << HOST << ":" << PORT << std::endl;

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

        if (bytesReceived <= 0) {
            continue;
        }

        std::cout << "> received: ( " << buffer << " ) from ( "
                  << inet_ntoa(clientAddr.sin_addr) << " , "
                  << ntohs(clientAddr.sin_port) << " )"
                  << std::endl;


        sendto(
            serverSocket,
            buffer,
            bytesReceived,
            0,
            (sockaddr*)&clientAddr,
            addrLen
        );

        
        if (std::string(buffer) == "quit") {
            std::cout << "> client sent quit, continuing server" << std::endl;
        }
    }

    close(serverSocket);
    std::cout << "> udp-echo-server is de-activated" << std::endl;

    return 0;
}

