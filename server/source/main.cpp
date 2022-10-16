#include "server.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Invalid command" << endl; 
    }

    string ip_address = argv[1];
    int port = stoi(argv[2]);
    
    ServerSocket server_socket(ip_address, port);
    server_socket.AcceptConnection();
    server_socket.Handle();
}