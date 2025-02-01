#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#define MY_SOCK_PATH "./sock_paths"
#define MYPORT 3490


int main(int argc, char * argv[]){
    int sockfd, client_fd;
    struct sockaddr_in my_addr; // 本地地址信息
    struct sockaddr_in client_addr; // 客户端连接的地址信息
    socklen_t addr_length;

    unlink(MY_SOCK_PATH); // 删除已存在的文件

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(MYPORT);
    my_addr.sin_addr.s_addr = INADDR_ANY; // 自动IP地址获取 inet_addr("127.0.0.1");

    bzero(&(my_addr.sin_zero), 8);

    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1){
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 10) == -1){
        perror("listen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    addr_length = sizeof(struct sockaddr_in); // 地址长度
    client_fd = accept(sockfd, (struct sockaddr *) &client_addr, &addr_length);
    if (client_fd == -1){
        perror("accept");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 写入和读取数据
    // int size;
    // char buffer[1024];
    // size = write(client_fd, "Hello, world!", 13);
    // size = read(client_fd, buffer, 1024);
    printf("success");
    close(client_fd);
    close(sockfd);

    // shutdown(sockfd, SHUT_RDWR); // 切断读写，关闭双向连接
}