#include "client.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Invalid command" << endl; 
    }

    string ip_address = argv[1];
    int port = stoi(argv[2]);
    
    Client client(ip_address, port);
    client.AcceptConnection();
    client.Handle();
}