#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <cstring>
#include <string>

#define PORT 8080

using namespace std;

char buffer[1000];

void store_to_file(char *data)    {
}

void find_top_proc(char *data, char *top_proc_data)    {
}

int main(int argc, char *argv[])  {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket<0) {
        cerr<<"Error creating the client socket\n";
        exit(EXIT_FAILURE);
    }

    sockaddr_in client_address;
    memset(&client_address, 0, sizeof(client_address));
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(PORT);
    client_address.sin_addr.s_addr = INADDR_ANY;

    // connecting to server
    int connection_status = connect(client_socket, (struct sockaddr*) &client_address, sizeof(client_address));
    if(connection_status<0) {
        cerr<<"Error connecting to socket\n";
        exit(EXIT_FAILURE);
    }

    char server_response[2000];

    while(1)    {
        int read_size = recv(client_socket, server_response, sizeof(server_response), 0);
        if(read_size>0) {
            store_to_file(server_response);

            char buffer[1000];
            find_top_proc(server_response, buffer);
            send(client_socket, buffer, strlen(buffer), 0);
            break;
        }
    }

    close(client_socket);
    return 0;
}