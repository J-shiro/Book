#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define PORT_SERV 8888
#define NUM_DATA 100
#define BUFF_LEN 256
#define LENGTH 1024
static char buff_send[LENGTH]; // 接收缓冲区

// 使用connect只能使用 read 和 write
static void udpclie_echo_with_connet(int s, struct sockaddr *to){
    char buff[BUFF_LEN] = "UDP TEST";
    connect(s, to, sizeof(*to)); // 连接

    ssize_t n = write(s, buff, BUFF_LEN); // 发送数据

    read(s, buff, n); // 接收数据
}

static void udpclie_echo(int s, struct sockaddr *to){
    char buff[BUFF_LEN] = "UDP TEST"; // 发送给服务器的测试数据
    struct sockaddr_in from; // 服务器地址
    socklen_t len = sizeof(*to); // 地址长度

    // 缓冲区溢出对策
    // sendto(s, buff, BUFF_LEN, 0, to, len); // buff内容发送给服务器
    int i = 0;
    for(i = 0; i < NUM_DATA; i++){
        *((int *)&buff_send[0]) = htonl(i); // 将数据标记打包，加入序号
        memcpy(&buff_send[4], buff, sizeof(buff));// 数据复制到发送缓冲区，前4个字节存储i
        sendto(s, &buff_send[0], NUM_DATA, 0, to, len);
    }

    recvfrom(s, buff, BUFF_LEN, 0, (struct sockaddr *)&from, &len);
    
    printf("recved: %s\n", buff);
}

int main(int argc, char * argv[]){
    int s;
    struct sockaddr_in addr_serv, addr_clie;

    s = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&addr_serv, 0, sizeof(addr_serv));
    addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_serv.sin_family = AF_INET;
    addr_serv.sin_port = htons(PORT_SERV);

    // connect 将套接字文件与远程地址端口关联起来
    // connect(s, (struct sockaddr *)&addr_serv, sizeof(addr_serv));
    // getsockname(s, &local, &len); // 获得套接字文件描述符地址
    // printf("udp local addr: %s\n", inet_ntoa(local.sin_addr))

    udpclie_echo(s, (struct sockaddr *)&addr_serv); // 客户端回显程序

    // 若客户端接收服务端第一次未接受完所有数据，之后数据会丢失，接收不到数据
    close(s);
    return 0;
}