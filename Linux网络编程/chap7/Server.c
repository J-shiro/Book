/*
框架:
    TCP server <-- read  write -- TCP client <-- read stdin
    TCP server -- write read --> TCP client --> write stdout
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
// #include<Linux/in.h>
#include <netinet/in.h>

#define PORT 8888
#define BACKLOG 2 // 侦听队列长度

void process_conn_server(int s){
    ssize_t size = 0;
    char buffer[1024]; // 数据缓冲区

    for(;;){
        size = read(s, buffer, 1024); // 从套接字中读取数据到缓冲区

        if(size == 0){
            return;
        }

        sprintf(buffer, "%zd bytes altogether\n", size);
        write(s, buffer, strlen(buffer) + 1); // 发送给客户端
    }
}

int main(int argc, char *argv[]){
    int ss, sc; // ss: server socket, sc: client socket
    struct sockaddr_in server_addr; // 服务器地址结构
    struct sockaddr_in client_addr; // 客户端地址结构
    int err; 
    pid_t pid;

    // 建立套接字
    ss = socket(AF_INET, SOCK_STREAM, 0);
    
    if(ss < 0){
        perror("socket error");
        return -1;
    }

    // 设置服务器地址
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 绑定地址到套接字描述符
    err = bind(ss, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(err < 0){
        perror("bind error\n");
        return -1;
    }

    // 设置侦听
    err = listen(ss, BACKLOG);
    if(err < 0){
        perror("listen error\n");
        return -1;
    }

    // 主循环
    for(;;){
        socklen_t addrlen = sizeof(struct sockaddr);

        sc = accept(ss, (struct sockaddr *)&client_addr, &addrlen);
        if (sc < 0){
            continue; // 出错结束本次循环
        }

        // 创建子进程
        pid = fork();
        if(pid == 0){ // 子进程
            close(ss); // 关闭服务器的侦听
            process_conn_server(sc); // 处理连接
        }else{ // 父进程
            close(sc); // 关闭客户端的连接
        }
    }
}