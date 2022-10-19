#include "server.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Invalid command" << endl;
        return -1;
    }
    
    string ip_address = argv[1];
    int port = stoi(argv[2]);

    Server server(ip_address, port);
    server.Communicate();
}