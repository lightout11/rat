#include "server.h"
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sstream>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <fstream>
#include <cstdio>
#include <netinet/tcp.h>

using namespace std;

const std::string Server::kOutput = "output.txt";
const std::string Server::kGetFile = "getfile";
const std::string Server::kSendFile = "sendfile";
const std::string Server::kRun = "run";

Server::Server() {
    cout << "Creating socket..." << endl;
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        cout << "Error creating socket: " << strerror(errno) << endl;
        exit(-1);
    }
}

Server::Server(string ip_address, int port) {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        cout << "Error creating socket: " << strerror(errno) << endl;
        exit(-1);
    }
    internet_socket_address_.sin_addr.s_addr = inet_addr(ip_address.c_str());
    internet_socket_address_.sin_family = AF_INET;
    internet_socket_address_.sin_port = htons(port);
    cout << "Connecting..." << endl;
    if (connect(socket_, (sockaddr*)&internet_socket_address_, sizeof(internet_socket_address_))) {
        cout << "Error connecting to server: " << strerror(errno) << endl;
        exit(-1);
    }
}

int Server::Connect(string ip_address, int port) {
    cout << "Connecting..." << endl;
    if (connect(socket_, (sockaddr*)&internet_socket_address_, sizeof(internet_socket_address_))) {
        cout << "Error connecting to server: " << strerror(errno) << endl;
        exit(-1);
    }
    cout << "Connected!" << endl;
    return 0;
}

int Server::SendMessage() {
    char buffer[kBufferSize + 1];
    memset(buffer, 0, kBufferSize + 1);
    strcpy(buffer, message_.c_str());
    if (send(socket_, buffer, kBufferSize, 0) < 0) {
        cout << "Error sending message: " << strerror(errno) << endl;
        return -1;
    }
    return 0;
}

int Server::ReceiveMessage() {
    message_.clear();
    char buffer[kBufferSize + 1];
    memset(buffer, 0, kBufferSize + 1);
    if (recv(socket_, buffer, kBufferSize, 0) < 0) {
        cout << "Error receiving message: " << strerror(errno) << endl;
    }
    message_ += buffer;
    return 0;
}

int Server::SendFile(string path) {
    message_ = path;
    SendMessage();
    int fd = open(path.c_str(), O_RDONLY);
    char buffer[kBufferSize + 1];
    struct stat filestat;
    if (stat(path.c_str(), &filestat) < 0) {
        cout << "Error getting file information: " << strerror(errno) << endl;
        return -1;
    }
    int filesize = filestat.st_size;
    sprintf(buffer, "%d", filesize);
    while (read(fd, buffer, kBufferSize)) {
        cout << "Sending data..." << endl;
        if (send(socket_, buffer, kBufferSize, 0) < 0) {
            cout << "Error sending data: " << strerror(errno) << endl;
            return -1;
        }
    }
    close(fd);
    cout << "Success!" << endl;
    return 0;
}

int Server::ReceiveFile() {
    cout << "Receive file from client..." << endl;
    // get file path
    ReceiveMessage();
    string path = message_;
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    char buffer[kBufferSize + 1];
    memset(buffer, 0, kBufferSize + 1);
    // get file size
    if (recv(socket_, buffer, kBufferSize, 0) < 0) {
        cout << "Error receiving file size: " << strerror(errno) << endl;
        return -1;
    }
    int filesize = atoi(buffer);
    int total_byte_received = 0;
    // get data
    while (total_byte_received < filesize) {
        memset(buffer, 0, kBufferSize + 1);
        int nbytes = recv(socket_, buffer, kBufferSize, 0);
        if (nbytes < 0) {
            cout << "Error receiving data: " << strerror(errno) << endl;
            return -1;
        }
        write(fd, buffer, nbytes);
        total_byte_received += nbytes;
    }
    close(fd);
    return 0;
}

int Server::Communicate() {
    while (true) {
        cout << "> ";
        getline(cin, message_);
        while (message_[0] == ' ') {
            message_.erase(0, 1);
        }
        while (message_.find("  ") != message_.npos) {
            message_.erase(message_.find("  "), 1);
        }
        while (*message_.end() == ' ') {
            message_.pop_back();
        }
        SendMessage();
        if (message_.find(kSendFile) == 0) {
            string path = message_.substr(message_.find(" "));
            SendFile(path);
        } else if (message_.find(kGetFile) == 0) {
            cout << "Get File!" << endl;
            if (ReceiveFile() == 0) {
                cout << "Success!" << endl; 
            } else {
                cout << "Failed!" << endl;
            }
        } else if (message_.find(kRun) == 0) {
            continue;
        } else {
            ReceiveFile();
            ifstream ifs(kOutput);
            string temp;
            while (getline(ifs, temp)) {
                cout << temp << endl;
            }
        }
    }
    return 0;
}