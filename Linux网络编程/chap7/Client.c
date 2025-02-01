#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<strings.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>

#define PORT 8888 // 设置侦听端口地址

void sig_pipe(int sign){
    printf("Catch a SIGPIPE signal\n");
    // 释放资源
}

void sig_int(int sign){
    printf("Catch a SigINT signal\n");
    exit(0);
}

void process_conn_client(int s){
    ssize_t size = 0;
    char buffer[1024]; // 数据缓冲区

    for(;;){
        size = read(0, buffer, 1024); // 从标准输入中读取数据
        if(size > 0){
            write(s, buffer, size); // 发送给服务器
            size = read(s, buffer, 1024); // 从服务器读取数据
            write(1, buffer, size); // 写到标准输出
        }
    }
}

int main(int argc, char *argv[]){
    
    signal(SIGPIPE,sig_pipe); // 捕捉SIGPIPE信号 服务器已关闭，客户端输入触发
    signal(SIGINT, sig_int); // 捕捉SIGINT信号 客户端Ctrl+C终止触发

    int s; // 客户端套接字描述符
    struct sockaddr_in server_addr; // 服务器地址结构
    int err;

    s = socket(AF_INET, SOCK_STREAM, 0); // 建立套接字
    if(s < 0){
        perror("socket error\n");
        return -1;
    }

    // 设置服务器地址
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 本地地址

    // 将用户输入的字符串类型的IP地址转换为整形
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    // 向服务器发起连接请求
    connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr));
    process_conn_client(s); // 客户端处理过程
    close(s);

}