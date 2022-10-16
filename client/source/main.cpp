#include "client.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Invalid command" << endl;
        return -1;
    }
    
    string ip_address = argv[1];
    int port = stoi(argv[2]);

    ClientSocket client_socket(ip_address, port);
    client_socket.Communicate();
}