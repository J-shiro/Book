#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<signal.h>
#include<unistd.h>

#define BUFFLEN 1024
#define SERVER_PORT 8888
#define BACKLOG 5
#define PIDNUMB 2

static void handle_connect(int s){
    struct sockaddr_in from; // 客户端地址
    socklen_t len = sizeof(from);
    int n = 0;
    char buff[BUFFLEN];
    time_t now;

    while(1){
        memset(buff, 0, BUFFLEN);
        // 接收客户端连接
        n = recvfrom(s, buff, BUFFLEN, 0, (struct sockaddr*)&from, &len);

        if(n > 0 && !strncmp(buff, "TIME", 4)){
            memset(buff, 0, BUFFLEN);
            now = time(NULL);
            sprintf(buff, "%24s\r\n", ctime(&now));
            sendto(s, buff, strlen(buff), 0, (struct sockaddr *)&from, len);
        }
    }
}

void sig_int(int num){
    exit(1);
}

int main(int argc, char *argv[]){
    int s_s; // 服务器套接字文件描述符
    struct sockaddr_in local; // 本地地址

    signal(SIGINT, sig_int);

    s_s = socket(AF_INET, SOCK_DGRAM, 0);

    // 初始化地址
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(SERVER_PORT);

    int err = bind(s_s, (struct sockaddr*)&local, sizeof(local));

    pid_t pid[PIDNUMB]; // 处理客户端连接
    int i = 0;
    for(i = 0; i < PIDNUMB; i++){
        pid[i] = fork();
        if(pid[i] == 0){
            // 子进程
            handle_connect(s_s);
        }
    }
    while(1);
    return 0;
}