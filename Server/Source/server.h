#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <sys/types.h>
#include <string>

class ServerSocket {
    public:
        ServerSocket();
        ServerSocket(std::string ip_address, int port);

        int AcceptConnection();
        int ReceiveMessage();
        int SendMessage();
        int ParseMessage();
        int Handle();

    protected:
        static constexpr int kDefaultPort = 12345;
        static constexpr int kMaxBacklog = 1;
        static constexpr int kBufferSize = 1024;
        static constexpr char kBreak = '\e';
        
        inline static const std::string kPwd = "pwd";
        inline static const std::string kGetFile = "getfile";
        inline static const std::string kSendFile = "sendfile";

    private:
        int socket_;
        int accepted_socket_;
        sockaddr_in internet_socket_address_;
        std::string message_;
};

#endif