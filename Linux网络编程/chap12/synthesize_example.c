#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <error.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/tcp.h>

#define PORT 8888
#define BACKLOG 8

static int alive = 1;

static void sigpipe(int signo)
{
    alive = 0;
}

int main(int argc, char *argv[])
{
    // s 为服务器的侦听套接字描述符， sc 为客户端连接成功返回的描述符
    int s, sc;

    // local_addr 本地地址， client_addr 客户端地址
    struct sockaddr_in local_addr, client_addr;
    int err = -1;          // 错误返回值
    socklen_t optlen = -1; // 整型的选项类型值
    int optval = -1;       // 选项类型值长度

    // 截取 SIGPIPE 和 SIGINT 由函数 signo 处理
    signal(SIGPIPE, sigpipe);
    signal(SIGINT, sigpipe);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1)
    {
        printf("套接字创建失败!\n");
        return -1;
    }

    // 设置地址和端口重用
    optval = 1; // 重用有效
    optlen = sizeof(optval);
    err = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, optlen);
    if (err == -1) // 设置失败
    {
        printf("套接字可重用设置失败!\n");
        return -1;
    }

    // 初始化本地协议族, 端口和 IP 地址
    bzero(&local_addr, sizeof(local_addr));  // 清理
    local_addr.sin_family = AF_INET;         // 协议族
    local_addr.sin_port = htons(PORT);       // 端口
    local_addr.sin_addr.s_addr = INADDR_ANY; // 任意本地地址

    // 绑定套接字
    err = bind(s, (struct sockaddr *)&local_addr, sizeof(struct sockaddr));
    if (err == -1)
    {
        printf("绑定失败!\n");
        return -1;
    }

    // 设置最大接收缓冲区和最大发送缓冲区
    optval = 128 * 1024;
    optlen = sizeof(optval);
    err = setsockopt(s, SOL_SOCKET, SO_RCVBUF, &optval, optlen);
    if (err == -1)
    {
        printf("设置接收缓冲区失败\n");
    }

    err = setsockopt(s, SOL_SOCKET, SO_SNDBUF, &optval, optlen);
    if (err == -1)
    {
        printf("设置发送缓冲区失败\n");
    }

    // 修改发送和接收超时时间
    struct timeval tv;
    tv.tv_sec = 1;       // 1s
    tv.tv_usec = 200000; // 200ms
    optlen = sizeof(tv);

    err = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, optlen);
    if (err == -1)
    {
        printf("设置接收超时时间失败\n");
    }

    err = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, optlen);
    if (err == -1)
    {
        printf("设置发送超时时间失败\n");
    }

    // 设置监听队列长度
    err = listen(s, BACKLOG);
    if (err == -1)
    {
        printf("设置监听失败\n");
        return -1;
    }

    // 设置 accept 超时时间
    printf("等待连接...");
    fd_set fd_r; // 读文件描述符集
    // struct timeval tv;
    tv.tv_usec = 200000;
    tv.tv_sec = 0;
    while (alive)
    {
        // 有连接请求时进行连接
        socklen_t sin_size = sizeof(struct sockaddr_in);

        // 轮询是否有客户连接到来
        FD_ZERO(&fd_r);
        FD_SET(s, &fd_r);
        switch (select(s + 1, &fd_r, NULL, NULL, &tv))
        {
        case -1: // 错误发生
        case 0:  // 超时
            continue;
            break;
        default: // 有连接到来
            break;
        }

        // 有连接到来, 接收...
        sc = accept(s, (struct sockaddr *)&client_addr, &sin_size);
        if (sc == -1)
        {
            perror("接收连接失败!\n");
            continue;
        }

        // 设置客户端超时探测时间
        optval = 10;
        optlen = sizeof(optval);
        err = setsockopt(sc, IPPROTO_TCP, SO_KEEPALIVE, (char *)&optval, optlen);
        if (err == -1)
        {
            printf("设置连接探测间隔时间失败\n");
        }

        // 禁止 Nagle 算法
        optval = 1;
        optlen = sizeof(optval);
        err = setsockopt(sc, IPPROTO_TCP, TCP_NODELAY, (char *)&optval, optlen);
        if (err == -1)
        {
            printf("禁止 Nagle 算法失败\n");
        }

        // 设置 linger
        struct linger linger;
        linger.l_onoff = 1;  // 延迟关闭生效
        linger.l_linger = 0; // 立即关闭
        optlen = sizeof(linger);
        err = setsockopt(sc, SOL_SOCKET, SO_LINGER, (char *)&linger, optlen);
        if (err == -1)
        {
            printf("设置立即关闭失败\n");
        }

        // 打印客户端 IP 地址信息
        err = send(sc, "连接成功!\n", 10, 0);
        if (err == -1)
        {
            printf("发送通知信息失败!\n");
        }

        // 关闭客户端
        close(sc);
    }

    close(s);
    return 0;
}
