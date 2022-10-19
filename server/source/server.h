#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <string>
#include <netinet/in.h>
#include <netinet/ip.h>

class Server {
    public:
        Server();
        Server(std::string ip_address, int port);

        int Connect(std::string ip_adress, int port);
        int SendMessage();
        int ReceiveMessage();
        int SendFile(std::string path);
        int ReceiveFile();
        int Communicate();

    protected:
        static constexpr int kBufferSize = 512;

        static const std::string kGetFile;
        static const std::string kSendFile;
        static const std::string kOutput;
        static const std::string kRun;

    private:
        int socket_;
        sockaddr_in internet_socket_address_;
        std::string message_;
};

#endif