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
    cout << "> echo-client is activated" << endl;

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        cout << "> socket() failed and program terminated" << endl;
        return 0;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(SERVER_PORT);

    if (::connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cout << "> connect() failed and program terminated" << endl;
        close(clientSocket);
        return 0;
    }

    cout << "> connected to the server" << endl;

    while (true)
    {
        cout << "> ";
        string sendMsg;
        getline(cin, sendMsg);

        send(clientSocket, sendMsg.c_str(), sendMsg.size(), 0);

        char recvBuffer[1024];
        memset(recvBuffer, 0, sizeof(recvBuffer));

        ssize_t recvLen = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
        if (recvLen <= 0) {
            cout << "> server closed the connection" << endl;
            break;
        }

        cout << "> received: " << recvBuffer << endl;

        if (sendMsg == "quit") {
            break;
        }
    }

    close(clientSocket);
    cout << "> echo-client is de-activated" << endl;

    return 0;
}

