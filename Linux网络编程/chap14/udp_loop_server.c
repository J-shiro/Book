// socket -> bind -> recvfrom -> 处理数据

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>

#define BUFFLEN 1024
#define SERVER_PORT 8888

int main(int argc, char *argv[]){
    int s; // 服务器套接字文件描述符
    struct sockaddr_in local, to; // 本地地址
    time_t now; // 时间
    char buff[BUFFLEN]; // 收发数据缓冲区
    int n = 0; 
    socklen_t len = sizeof(to); // 地址长度

    s = socket(AF_INET, SOCK_DGRAM, 0); // 创建UDP套接字

    // 初始化地址
    memset(&local, 0, sizeof(local)); // 清零
    local.sin_family = AF_INET; // 协议族
    local.sin_addr.s_addr = htonl(INADDR_ANY); // 本地地址
    local.sin_port = htons(SERVER_PORT); // 本地端口

    int err = bind(s, (struct sockaddr *)&local, sizeof(local)); 
    
    while(1){
        memset(buff, 0, BUFFLEN); // 清零
        n = recvfrom(s, buff, BUFFLEN, 0, (struct sockaddr *)&to, &len); // 接收数据

        if(n > 0 && !strncmp(buff, "TIME", 4)){
            memset(buff, 0, BUFFLEN); // 清零
            now = time(NULL); // 获取当前时间
            sprintf(buff, "%24s\r\n", ctime(&now)); // 格式化时间
            sendto(s, buff, strlen(buff), 0, (struct sockaddr *)&to, len); // 发送数据
        }
    }

    close(s);
    return 0;

}