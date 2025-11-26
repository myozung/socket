#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 65456

int main()
{
    cout << "> echo-server is activated" << endl;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cout << "> socket() failed and program terminated" << endl;
        return 0;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(SERVER_PORT);

    if (::bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cout << "> bind() failed and program terminated" << endl;
        close(serverSocket);
        return 0;
    }

    if (::listen(serverSocket, 5) < 0) {
        cout << "> listen() failed and program terminated" << endl;
        close(serverSocket);
        return 0;
    }

    cout << "> server is ready" << endl;

    while (true)
    {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            cout << "> accept() failed" << endl;
            continue;
        }

        cout << "> client connected by IP address "
             << inet_ntoa(clientAddr.sin_addr)
             << " with Port number "
             << ntohs(clientAddr.sin_port)
             << endl;

        while (true)
        {
            char recvBuffer[1024];
            memset(recvBuffer, 0, sizeof(recvBuffer));

            ssize_t recvLen = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
            if (recvLen <= 0) break;

            cout << "> echoed: " << recvBuffer << endl;

            send(clientSocket, recvBuffer, recvLen, 0);

            if (strcmp(recvBuffer, "quit") == 0) {
                break;
            }
        }

        close(clientSocket);
    }

    close(serverSocket);
    cout << "> echo-server is de-activated" << endl;

    return 0;
}

