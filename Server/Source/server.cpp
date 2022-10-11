#include "server.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <iostream>
#include <errno.h>
#include <cstring>
#include <arpa/inet.h>
#include <sstream>
#include <cstring>

using namespace std;

ServerSocket::ServerSocket() {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        cout << "Error creating socket: " << strerror(errno) << endl;
        exit(-1);
    }
    sockaddr_in internet_socket_address;
    internet_socket_address.sin_addr.s_addr = INADDR_ANY;
    internet_socket_address.sin_family = AF_INET;
    internet_socket_address.sin_port = kDefaultPort;
    if (bind(socket_, (sockaddr*)&internet_socket_address, sizeof(internet_socket_address))) {
        cout << "Error binding socket: " << strerror(errno) << endl;
        exit(-1);
    }
}

ServerSocket::ServerSocket(string ip_address, int port) {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        cout << "Error creating socket: " << strerror(errno) << endl;
        exit(-1);
    }
    internet_socket_address_.sin_addr.s_addr = inet_addr(ip_address.c_str());
    internet_socket_address_.sin_family = AF_INET;
    internet_socket_address_.sin_port = htons(port);
    if (bind(socket_, (sockaddr*)&internet_socket_address_, sizeof(internet_socket_address_))) {
        cout << "Error binding socket: " << strerror(errno) << endl;
        exit(-1);
    }
}

int ServerSocket::AcceptConnection() {
    if (listen(socket_, kMaxBacklog) < 0) {
        cout << "Error listening for connection: " << strerror(errno) << endl;
        exit(-1);
    }
    accepted_socket_ = accept(socket_, (sockaddr*)&internet_socket_address_, (socklen_t*)sizeof(internet_socket_address_));
    if (accepted_socket_ < 0) {
        cout << "Error accepting socket: " << strerror(errno) << endl;
        exit(-1);
    }
}

int ServerSocket::ReceiveMessage() {
    message_.clear();
    char buffer[kBufferSize];
    while (true) {
        recv(accepted_socket_, buffer, kBufferSize, 0);
        if (strchr(buffer, kBreak) != nullptr) {
            break;
        }
        message_ += buffer;
    }
}

int ServerSocket::SendMessage() {
    stringstream ss;
    ss << message_;
    char buffer[kBufferSize];
    memset(buffer, 0, kBufferSize);
    while (ss.get(buffer, kBufferSize)) {
        send(accepted_socket_, buffer, kBufferSize, 0);
        memset(buffer, 0, kBufferSize);
    }
}