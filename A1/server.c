#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<dirent.h>
#include<ctype.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>

#define N 5
#define PORT 8080
#define QUERYMSG "query"

typedef struct proc_info    {
    char pname[30];
    int cpu_time;
};

int is_pid(struct dirent *entry);
int sorter(const void *p, const void *q);
void *connection_handler(void *);
void find_process_info(char *data);
void top_n_procs(char *data);

int main(int argc , char *argv[])   {
    int socket_desc , new_socket , c , *new_sock;
    struct sockaddr_in server , client;
    char *message;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)  {
        perror("Could not create socket");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // binding
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)  {
        perror("bind failed");
        return 1;
    }
    puts("bind done");

    // listening for clients
    listen(socket_desc , 10);

    // accept incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ) {
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
        perror("could not accept");
        return 1;
    }

    return 0;
}


void *connection_handler(void *socket_desc) {
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
    DIR *proc_dir;
    struct dirent *entry;
    int fd;

    proc_dir = opendir("/proc");
    if(proc_dir == NULL)    {
        perror("could not open directory");
        return;
    }

    // finding number of current processes
    int count = 0;
    while(entry = readdir(proc_dir))    {
        if(!is_pid(entry))  {
            continue;
        }
        count++;
    }
    struct proc_info proc_array[count + 100];

    int i = 0;
    proc_dir = opendir("/proc");

    while((entry = readdir(proc_dir)))  {
        if(!is_pid(entry))  {
            continue;
        }
        char path[300] = "/proc/";
        strcat(path, entry->d_name);
        strcat(path, "/stat");

        fd = open(path, O_RDONLY);

        if(fd == -1)    {
            perror("cant open file\n");
            return;
        }

        // count 13 whitespaces
        int whites = 0, l1 = 0, l2 = 0, l3 = 0;
        char buffer[200], name[30], num1[15], num2[15];
        read(fd, buffer, sizeof(buffer));

        int j;
        for(j=0; j < sizeof(buffer); j++)   {
            if(buffer[j] == ' ')    {
                whites++;
                continue;
            }
            if(whites == 1) {
                name[l1++] = buffer[j];
            }
            if(whites == 13)    {
                num1[l2++] = buffer[j];
            }
            if(whites == 14)    {
                num2[l3++] = buffer[j];
            }
        }
        name[l1] = '\0';
        num1[l2] = '\0';
        num2[l3] = '\0';

        int runtime = atoi(num1) + atoi(num2);
        strcpy(proc_array[i].pname, name);
        proc_array[i].cpu_time = runtime;
    }

    qsort((void*)proc_array, count+100, sizeof(proc_array[0]), sorter);

    for(int i=0; i<N; i++)  {
        char num[15];
        sprintf(num, "%d", proc_array[i].cpu_time);
        if(i==0)    {
            strcpy(data, proc_array[i].pname);
            strcat(data, " : ");
            strcat(data, num);
            strcat(data, "\n");
        }
        else    {
            strcat(data, proc_array[i].pname);
            strcat(data, " : ");
            strcat(data, num);
            strcat(data, "\n");
        }
    }
}

int is_pid(struct dirent *entry)    {
    char *name = entry -> d_name;
    for(int i=0; i<strlen(name) ; i++)  {
        if(!isdigit(name[i])) {
            return 0;
        }
    }
    return 1;
}

int sorter(const void *p, const void *q)    {
    if(!p && !q)    return 0;
    if(!p)          return 1;
    if(!q)          return -1;

    int time1 = ((struct proc_info *)p)->cpu_time;
    int time2 = ((struct proc_info *)q)->cpu_time;

    return time2 - time1;
}