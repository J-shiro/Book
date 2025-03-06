#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>

#define BUFFLEN 1024
#define SERVER_PORT 8888

int main(int argc, char *argv[]){
    int s;
    struct sockaddr_in server;
    char buff[BUFFLEN];
    int n = 0;

    // 建立 TCP 套接字
    s = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY); // 任意本地地址
    server.sin_port = htons(SERVER_PORT); // 服务器端口


    // 连接服务器
    int err = connect(s, (struct sockaddr*)&server, sizeof(server));

    memset(buff, 0, BUFFLEN); // 清空缓冲区
    strcpy(buff, "TIME"); // 发送请求字符串
    send(s, buff, strlen(buff), 0); // 发送请求
    memset(buff, 0, BUFFLEN); // 清空缓冲区

    n = recv(s, buff, BUFFLEN, 0); // 接收服务器响应
    if(n > 0){
        printf("TIME: %s\n", buff); // 打印服务器响应
    }

    close(s);
    return 0;
}