#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<arpa/inet.h>

int main(int argc, char *argv[]){
    int s;
    struct sockaddr_in from; // 发送方地址信息
    struct sockaddr_in local; // 本地地址信息

    int from_len = sizeof(from); // 地址结构的长度
    int n; // 接收到的数据长度
    char buf[128];

    // 初始化IPv4族的数据报套接字
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if(s == -1){
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(8888);

    // 套接字绑定
    bind(s, (struct sockaddr *)&local, sizeof(local));

    // recvfrom接收数据, 从s接收到buf中
    // 所接受数据的来源（发送数据的主机IP地址，端口等信息）可从from变量中获得
    n = recvfrom(s, buf, 128, 0, (struct sockaddr *)&from, &from_len);
    if(n == -1){
        perror("recvfrom error");
        exit(EXIT_FAILURE);
    }

    // 处理数据
}