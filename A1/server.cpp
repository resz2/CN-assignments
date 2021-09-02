#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>
#include <cstring>
#include <pthread.h>

#define PORT 8080
#define CLIENTS 20
#define N 5

using namespace std;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char client_message[1000];

void find_process_info(char *data, int len) {
}

void* thread_func(void *arg) {
    int new_socket = *((int*)arg);
    while(1)    {
        int read_size = recv(new_socket, client_message, sizeof(client_message), 0);
        if(read_size>0) {
            break;
        }

        char buffer[2000];
        //pthread_mutex_lock(&mutex);
        find_process_info(buffer, sizeof(buffer));
        //pthread_mutex_unlock(&mutex);
        // send process info back to client
        send(new_socket, buffer, strlen(buffer), 0);
        memset(&buffer, 0, sizeof(buffer));
    }
    close(new_socket);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])  {

    struct sockaddr_storage server_storage;
    socklen_t address_size;

    // Creating the socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket<0) {
        cerr<<"Error creating the server socket\n";
        exit(EXIT_FAILURE);
    }

    // defining server address
    sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // binding socket to local address
    int bind_status = bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    if(bind_status<0)   {
        cerr<<"Error in binding socket\n";
        exit(EXIT_FAILURE);
    }

    // listening for client
    // a set number of clients are allowed to connect at a time
    int listen_status = listen(server_socket, CLIENTS);
    if(listen_status<0) {
        cerr<<"Error in listening\n";
        exit(EXIT_FAILURE);
    }

    pthread_t threads[CLIENTS+10];
    int i = 0;

    while(1)    {
        // new socket for incoming connection
        address_size = sizeof(server_storage);
        int new_socket = accept(server_socket, (struct sockaddr *)&server_storage, &address_size);

        // creating new thread for each client
        if(pthread_create(&threads[i++], NULL, thread_func, &new_socket) != 0)  {
            cerr<<"Eror in thread creation\n";
        }

        if(i >= CLIENTS)   {
            i = 0;
            while(i < CLIENTS)  {
                pthread_join(threads[i++], NULL);
            }
            i = 0;
        }
    }

    //close(server_socket);
    return 0;
}