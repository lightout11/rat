#ifndef CLIENT_H
#define CLIENT_H

#include <sys/socket.h>
#include <string>
#include <netinet/in.h>
#include <netinet/ip.h>

class ClientSocket {
    public:
        ClientSocket();
        ClientSocket(std::string ip_address, int port);

        int Connect(std::string ip_adress, int port);
        int SendMessage();
        int ReceiveMessage();
        int SendFile(std::string path);
        int ReceiveFile();
        int Communicate();

    protected:
        static constexpr int kBufferSize = 512;

        inline static const std::string kGetFile = "getfile";
        inline static const std::string kSendFile = "sendfile";
        inline static const std::string kOutput = "output.txt";

    private:
        int socket_;
        sockaddr_in internet_socket_address_;
        std::string message_;
};

#endif