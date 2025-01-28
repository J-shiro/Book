#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#define MY_SOCK_PATH "./sock_paths"
#define UNIX_PATH_MAX 108

// 已经定义在/usr/include/sys/un.h
// struct sockaddr_un{
//     sa_family_t sun_family; // 协议族， AF_UNIX
//     char sun_path[UNIX_PATH_MAX]; // 路径名，最长108个
// };

int main(int argc, char * argv[]){
    int sfd;
    struct sockaddr_un addr;

    unlink(MY_SOCK_PATH); // 删除已存在的文件

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sfd == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, MY_SOCK_PATH, sizeof(addr.sun_path) - 1);
    if(bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1){
        perror("bind");
        close(sfd);
        exit(EXIT_FAILURE);
    }

    printf("Socket bound to %s\n", MY_SOCK_PATH);

    close(sfd);
}