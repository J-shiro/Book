#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    int err = -1;     // 返回值
    int s = -1;       // socket 描述符
    int snd_size = 0; // 发送缓冲区大小
    int rcv_size = 0; // 接收缓冲区大小
    socklen_t optlen; // 选项值长度

    // 建立 TCP 套接字
    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s == -1)
    {
        printf(" 建立套接字错误\n");
        return -1;
    }

    // 读取缓冲区设置情况，获取原始发送缓冲区大小
    optlen = sizeof(snd_size);
    err = getsockopt(s, SOL_SOCKET, SO_SNDBUF, &snd_size, &optlen);
    if (err)
    {
        printf("获取发送缓冲区大小错误\n");
    }

    // 获取原始接收缓冲区大小
    optlen = sizeof(rcv_size);
    err = getsockopt(s, SOL_SOCKET, SO_RCVBUF, &rcv_size, &optlen);
    if (err)
    {
        printf("获取接收缓冲区大小错误\n");
    }
    // 打印原始缓冲区设置情况
    printf(" 发送缓冲区原始大小为: %d 字节\n", snd_size);
    printf(" 接收缓冲区原始大小为: %d 字节\n", rcv_size);

    // 设置发送缓冲区大小
    snd_size = 4096;
    optlen = sizeof(snd_size);
    err = setsockopt(s, SOL_SOCKET, SO_SNDBUF, &snd_size, optlen);
    if (err)
    {
        printf("设置发送缓冲区大小错误\n");
    }

    // 设置接收缓冲区大小
    rcv_size = 8192;
    optlen = sizeof(rcv_size);
    err = setsockopt(s, SOL_SOCKET, SO_RCVBUF, &rcv_size, optlen);
    if (err)
    {
        printf("设置接收缓冲区大小错误\n");
    }

    // 检查修改情况
    // 获取修改后发送缓冲区大小
    optlen = sizeof(snd_size);
    err = getsockopt(s, SOL_SOCKET, SO_SNDBUF, &snd_size, &optlen);
    if (err)
    {
        printf("获取发送缓冲区大小错误\n");
    }

    // 获取修改后接收缓冲区大小
    optlen = sizeof(rcv_size);
    err = getsockopt(s, SOL_SOCKET, SO_RCVBUF, &rcv_size, &optlen);
    if (err)
    {
        printf("获取接受缓冲区大小错误\n");
    }

    // 结果打印
    printf(" 发送缓冲区大小为: %d 字节\n", snd_size);
    printf(" 接收缓冲区大小为: %d 字节\n", rcv_size);

    close(s);
    return 0;
}