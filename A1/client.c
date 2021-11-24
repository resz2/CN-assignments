#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define PORT 8080
#define QUERYMSG "query"

void find_top_proc(char *message, char *data);

int main(int argc , char *argv[])   {
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1) {
        perror("Could not create socket");
        return 1;
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)    {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    while(1) {
        send(sock , QUERYMSG , strlen(QUERYMSG) , 0);

        if( recv(sock , server_reply , 2000 , 0) < 0)   {
            perror("recv failed");
            return 1;
        }

        puts("top process info received\n");


        // // creating new file for each client
        // int pid = getpid();
        // char *mypid = malloc(6);
        // sprintf(mypid, "%d", pid);
        // puts(mypid);
        // char filename[30] = "client_files/cfile";
        // strcat(filename, mypid);
        // strcat(filename, ".txt");

        // // storing process info on local file
        // FILE *fp;
        // fp = fopen(filename, "w");
        // if( fp==NULL )  {
        //     perror("File failed to open\n");
        //     return;
        // }
        // if(strlen(server_reply)>0)  {
        //     fputs(server_reply, fp);
        //     fputs("\n", fp);
        // }
        // fclose(fp);

        // // finding top process
        // find_top_proc(message, filename);
        // send(sock , message , strlen(message) , 0);
        usleep(10000);
    }
    close(sock);
    return 0;
}

void find_top_proc(char *message, char *path)   {
    FILE *fd;
    fd = fopen(path, "r");
    fscanf(fd, "%[^\n]", message);
}