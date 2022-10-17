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
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cstdlib>
#include <netinet/tcp.h>

using namespace std;

extern char** environ;

ServerSocket::ServerSocket() {
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

ServerSocket::ServerSocket(string ip_address, int port) {
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

int ServerSocket::AcceptConnection() {
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
    // int optval = 1;
    // socklen_t optlen = sizeof(optval);
    // setsockopt(accepted_socket_, IPPROTO_TCP, TCP_NODELAY, &optval, optlen);
    cout << "Accepted!" << endl;
    return 0;
}

int ServerSocket::ReceiveMessage() {
    message_.clear();
    char buffer[kBufferSize + 1];
    memset(buffer, 0, kBufferSize + 1);
    if (recv(accepted_socket_, buffer, kBufferSize, 0) < 0) {
        cout << "Error receiving message: " << strerror(errno) << endl;
    }
    message_ += buffer;
    return 0;
}

int ServerSocket::SendMessage() {
    char buffer[kBufferSize + 1];
    memset(buffer, 0, kBufferSize + 1);
    strcpy(buffer, message_.c_str());
    cout << strlen(buffer) << endl;
    if (send(accepted_socket_, buffer, kBufferSize, 0) < 0) {
        cout << "Error sending message: " << strerror(errno) << endl;
        return -1;
    }
    return 0;
}

int ServerSocket::SendFile(string path) {
    // send file path
    message_ = path;
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
    sprintf(buffer, "%d", filesize);
    cout << buffer << endl;
    // send file size
    if (send(accepted_socket_, buffer, kBufferSize, 0) < 0) {
        cout << "Error sending file size: " << strerror(errno) << endl;
        return -1;
    }
    while (true) {
        int readdata = read(fd, buffer, kBufferSize);
        cout << readdata << endl;
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

int ServerSocket::ReceiveFile() {
    ReceiveMessage();
    string path = message_;
    cout << path << endl;
    int fd = open(path.c_str(), O_WRONLY);
    char buffer[kBufferSize + 1];
    recv(accepted_socket_, buffer, kBufferSize, 0);
    int filesize = atoi(buffer);
    int total_byte_received = 0;
    while (total_byte_received < filesize) {
        cout << "Receiving data..." << endl;
        int nbytes = 0;
        if (nbytes = recv(accepted_socket_, buffer, kBufferSize, 0) < 0) {
            cout << "Error receiving data: " << strerror(errno) << endl;
            return -1;
        }
        write(fd, buffer, kBufferSize);
        total_byte_received += nbytes;
    }
    close(fd);
    cout << "Success!" << endl;
    return 0;
}

int ServerSocket::Handle() {
    while (true) {
        cout << "> ";
        ReceiveMessage();
        cout << message_ << endl;
        if (message_.find(kSendFile) == 0) {
            ReceiveFile();
        } else if (message_.find(kGetFile) == 0) {            
            cout << "Get File!" << endl;
            string path = message_.substr(message_.find(" ") + 1);
            cout << path << endl;
            SendFile(path);
        } else if (message_.find(kRun) == 0) {
            fork();
            string command = message_.substr(message_.find(" ") + 1);
            execl(command.c_str(), NULL);
        } else {
            string command = message_ + " > " + kOutput;
            system(command.c_str());
            SendFile(kOutput);
        }
    }
    return 0;
}