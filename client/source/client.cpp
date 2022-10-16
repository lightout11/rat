#include "client.h"
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

using namespace std;

ClientSocket::ClientSocket() {
    cout << "Creating socket..." << endl;
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        cout << "Error creating socket: " << strerror(errno) << endl;
        exit(-1);
    }
}

ClientSocket::ClientSocket(string ip_address, int port) {
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

int ClientSocket::Connect(string ip_address, int port) {
    internet_socket_address_.sin_addr.s_addr = inet_addr(ip_address.c_str());
    internet_socket_address_.sin_family = AF_INET;
    internet_socket_address_.sin_port = htons(port);
    cout << "Connecting..." << endl;
    if (connect(socket_, (sockaddr*)&internet_socket_address_, sizeof(internet_socket_address_))) {
        cout << "Error connecting to server: " << strerror(errno) << endl;
        exit(-1);
    }
    cout << "Connected!" << endl;
    return 0;
}

int ClientSocket::SendMessage() {
    char buffer[kBufferSize + 1];
    strcpy(buffer, message_.c_str());
    // cout << buffer << endl;
    if (send(socket_, buffer, kBufferSize, 0) < 0) {
        cout << "Error sending message: " << strerror(errno) << endl;
        return -1;
    }
    return 0;
}

int ClientSocket::ReceiveMessage() {
    message_.clear();
    char buffer[kBufferSize + 1];
    if (recv(socket_, buffer, kBufferSize, 0) < 0) {
        cout << "Error receiving message: " << strerror(errno) << endl;
    }
    message_ += buffer;
    return 0;
}

int ClientSocket::SendFile(string path) {
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
        cout << buffer << endl;
    }
    close(fd);
    cout << "Success!" << endl;
    return 0;
}

// int ClientSocket::ReceiveFile() {
//     // receive file path
//     ReceiveMessage();
//     string path = message_;
//     cout << path << endl;
//     remove(path.c_str());
//     int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
//     char buffer[kBufferSize + 1];
//     // get file size
//     memset(buffer, 0, kBufferSize + 1);
//     if (recv(socket_, buffer, kBufferSize, 0) < 0) {
//         cout << "Error receiving file size: " << strerror(errno) << endl;
//         return -1;
//     }
//     cout << buffer << endl;
//     int filesize = atoi(buffer);
//     cout << filesize << endl;
//     int total_byte_received = 0;
//     while (total_byte_received < filesize) {
//         cout << "Receiving data..." << endl;
//         memset(buffer, 0, kBufferSize + 1);
//         int nbytes = recv(socket_, buffer, kBufferSize, 0);
//         if (nbytes < 0) {
//             cout << "Error receiving data: " << strerror(errno) << endl;
//             return -1;
//         }
//         // cout << nbytes << endl;
//         // cout << buffer << endl;
//         if (write(fd, buffer, kBufferSize) < 0) {
//             cout << "Error writing file: " << strerror(errno) << endl;
//             return -1;
//         }
//         total_byte_received += nbytes;
//         cout << total_byte_received << endl;
//     }
//     close(fd);
//     cout << "Success!" << endl;
//     return 0;
// }

int ClientSocket::ReceiveFile() {
    // get file path
    ReceiveMessage();
    string path = message_;
    cout << path << endl;
    FILE* file = fopen(path.c_str(), "w");
    char buffer[kBufferSize + 1];
    memset(buffer, 0, kBufferSize + 1);
    // get file size
    if (recv(socket_, buffer, kBufferSize, 0) < 0) {
        cout << "Error receiving file size: " << strerror(errno) << endl;
        return -1;
    }
    // cout << buffer << endl;
    int filesize = atoi(buffer);
    cout << filesize << endl;
    int total_byte_received = 0;
    // get data
    while (total_byte_received < filesize) {
        cout << "Receiving data..." << endl;
        memset(buffer, 0, kBufferSize + 1);
        int nbytes = recv(socket_, buffer, kBufferSize, 0);
        if (nbytes < 0) {
            cout << "Error receiving data: " << strerror(errno) << endl;
            return -1;
        }
        // cout << nbytes << endl;
        // cout << buffer << endl;
        fputs(buffer, file);
        total_byte_received += nbytes;
        cout << total_byte_received << endl;
    }
    fclose(file);
    cout << "Success!" << endl;
    return 0;
}

int ClientSocket::Communicate() {
    while (true) {
        cout << "> ";
        getline(cin, message_);
        // while (message_[0] == ' ') {
        //     message_.erase(0, 1);
        // }
        // while (message_.find("  ") != message_.npos) {
        //     message_.erase(message_.find("  "), 1);
        // }
        // while (*message_.end() == ' ') {
        //     message_.pop_back();
        // }
        SendMessage();
        if (message_.find(kSendFile) == 0) {
            string path = message_.substr(message_.find(" "));
            SendFile(path);
        } else if (message_.find(kGetFile) == 0) {
            cout << "Get File!" << endl;
            ReceiveFile();
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