#ifndef CLIENT_H
#define CLIENT_H

#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <netinet/in.h>
#include <netinet/ip.h>

class Client {
    public:
        Client();
        Client(std::string ip_address, int port);

        int AcceptConnection();
        int ReceiveMessage();
        int SendMessage();
        int Handle();
        int SendFile(std::string path);
        int ReceiveFile();

    protected:
        static constexpr int kDefaultPort = 12345;
        static constexpr int kMaxBacklog = 4;
        static constexpr int kBufferSize = 512;
                
        static const std::string kGetFile;
        static const std::string kSendFile;
        static const std::string kOutput;
        static const std::string kRun;

    private:
        int socket_;
        int accepted_socket_;
        sockaddr_in internet_socket_address_;
        std::string message_;
};

#endif