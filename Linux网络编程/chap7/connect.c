#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <string.h>
#define DEST_IP "172.16.185.1" // 服务器IP地址
#define DEST_PORT 23 // 服务器端口

int main(int argc, char * argv[]){

    int ret = 0;
    int sockfd;
    struct sockaddr_in server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd < 0){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(DEST_PORT);
    server.sin_addr.s_addr = inet_addr(DEST_IP);
    bzero(&(server.sin_zero), 8);

    ret = connect(sockfd, (struct sockaddr *) &server, sizeof(struct sockaddr));
    if(ret == -1){
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    close(sockfd);
}
