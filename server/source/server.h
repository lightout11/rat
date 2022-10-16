#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <netinet/in.h>
#include <netinet/ip.h>

class ServerSocket {
    public:
        ServerSocket();
        ServerSocket(std::string ip_address, int port);

        int AcceptConnection();
        int ReceiveMessage();
        int SendMessage();
        int ParseMessage();
        int Handle();
        int SendFile(std::string path);
        int ReceiveFile();
        int ExecuteCommand();
        int ChangeDirectory();
        int GetDirectory();

    protected:
        static constexpr int kDefaultPort = 12345;
        static constexpr int kMaxBacklog = 4;
        static constexpr int kBufferSize = 1024;
                
        inline static const std::string kGetFile = "getfile";
        inline static const std::string kSendFile = "sendfile";
        inline static const std::string kOutput = "output.txt";

    private:
        int socket_;
        int accepted_socket_;
        sockaddr_in internet_socket_address_;
        std::string message_;
};

#endif