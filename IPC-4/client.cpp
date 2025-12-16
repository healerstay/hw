#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>

int sock;
int running = 1;

void *receive(void *arg) {
    char buf[2048];
    while(running && recv(sock, buf, 2048, 0) > 0) {
        printf("%s", buf);
        memset(buf, 0, 2048);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Usage: %s ip port\n", argv[0]);
        return 1;
    }
    
    struct sockaddr_in addr;
    pthread_t tid;
    char name[50], buf[2048];
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));
    
    if(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Connection failed\n");
        return 1;
    }
    
    printf("Enter name: ");
    read(0, name, 50);
    send(sock, name, strlen(name), 0);
    
    printf("Connected\n\n");
    
    pthread_create(&tid, NULL, receive, NULL);
    
    while(running) {
        read(0, buf, 2048);
        send(sock, buf, strlen(buf), 0);
        
        if(strncmp(buf, "/exit", 5) == 0) {
            running = 0;
        }
        memset(buf, 0, 2048);
    }
    
    close(sock);
    return 0;
}

