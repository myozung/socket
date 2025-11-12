#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    const char* HOST = "127.0.0.1";
    const int PORT = 65456;
    const int BUFFER_SIZE = 1024;

    std::cout << "> echo-server is activated" << std::endl;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "socket creation failed" << std::endl;
        return 1;
    }
    
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
        std::cerr << "setsockopt failed" << std::endl;
        close(serverSocket);
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

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "listen failed" << std::endl;
        close(serverSocket);
        return 1;
    }
    
    std::cout << "> Listening on " << HOST << ":" << PORT << std::endl;

    sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == -1) {
        std::cerr << "accept failed" << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "> client connected by IP address "
              << inet_ntoa(clientAddr.sin_addr)
              << " with Port number "
              << ntohs(clientAddr.sin_port) << std::endl;
    
    char buffer[BUFFER_SIZE];
    while(true){
        std::memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytesReceived = read(clientSocket, buffer, BUFFER_SIZE - 1);
        if (bytesReceived <= 0){
            std::cout << "> client disconnected" << std::endl;
            break;
        }
        
        std::cout << "Client:" << buffer << std::endl;
        
        ssize_t bytesSent = write(clientSocket, buffer, bytesReceived);
        if (bytesSent == -1){
            std::cerr << "send failed" << std::endl;
            break;
        }
    }

    close(clientSocket);
    close(serverSocket);
    std::cout << "> echo-server is de-activated" << std::endl;

    return 0;
}
