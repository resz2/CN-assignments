#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define PORT 8080
#define QUERYMSG "query"

int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    send(sock , QUERYMSG , strlen(QUERYMSG) , 0);

    if( recv(sock , server_reply , 2000 , 0) < 0)   {
        perror("recv failed");
        return 1;
    }

    puts("process info received\n");
    // storing process info on local file
    FILE *fp;
    fp = fopen("client_file.txt", "w");
    if( fp==NULL )  {
        perror("File failed to open\n");
        return;
    }
    if(strlen(server_reply)>0)  {
        fputs(server_reply, fp);
        fputs("\n", fp);
    }
    fclose(fp);

    strcpy(message, "top proc");
    send(sock , message , strlen(message) , 0);

    close(sock);
    return 0;
}