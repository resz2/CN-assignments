#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>

#include<pthread.h>

#define N 5
#define PORT 8080
#define QUERYMSG "query"

typedef struct proc_info    {
    int pid;
    long long cpu_time;
};

int sorter(const void *p, const void *q);
void *connection_handler(void *);
void find_process_info(char *data);
void top_n_procs(char *data);

int main(int argc , char *argv[])
{
    int socket_desc , new_socket , c , *new_sock;
    struct sockaddr_in server , client;
    char *message;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)  {
        perror("Could not create socket");
        return 1;
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)  {
        perror("bind failed");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;

        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)   {
            perror("could not create thread");
            return 1;
        }

        puts("Handler assigned");
        // removed join for concurrent functioning
        //pthread_join( sniffer_thread , NULL);
    }

    if (new_socket<0)   {
        perror("accept failed");
        return 1;
    }

    return 0;
}

// This will handle connection for each client

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char buffer[2000] , client_message[2000];


    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )   {
        if(strcmp(client_message, QUERYMSG)==0)  {
            printf("Finding process info\n");
            find_process_info(buffer);

            // sending top N processes
            write(sock, buffer, strlen(buffer));
            memset(client_message, 0, strlen(client_message));
            continue;
        }
        // Print the top process
        printf("%s\n", client_message);
        //write(sock , client_message , strlen(client_message));
        memset(client_message, 0, strlen(client_message));
    }

    if(read_size == 0)  {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)    {
        perror("receive failed");
    }

    free(socket_desc);
    return 0;
}

void find_process_info(char *data)  {
    strcpy(data, "procinfo");

    top_n_procs(data);

    FILE *fp;
    fp = fopen("server_file.txt", "w");
    if( fp==NULL )  {
        perror("File failed to open\n");
        return;
    }
    if(strlen(data)>0)  {
        fputs(data, fp);
        fputs("\n", fp);
    }
    fclose(fp);
}

void top_n_procs(char *data)   {
}

int sorter(const void *p, const void *q)    {
    long long time1 = ((struct proc_info *)p)->cpu_time;
    long long time2 = ((struct proc_info *)q)->cpu_time;

    return time2 - time1;
}