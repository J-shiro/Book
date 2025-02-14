#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<string.h>
#include<stdio.h>
#include<errno.h>
#include<unistd.h>
#include<stdlib.h>

// 进程B获得进程A中发送过来的消息，并从中获取文件描述符，根据文件描述符直接从文件中读取数据，并将其在标准输出打印出来

// 从fd接收消息，并将文件描述符放在指针recvfd中
ssize_t recv_fd(int fd, void * data, size_t bytes, int * recvfd){
    struct msghdr msghdr_recv; // 接收消息结构
    struct iovec iov[1]; // 接收消息的向量
    size_t n;
    int newfd;

    union{
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    }control_un;

    struct cmsghdr * pcmsghdr; // 消息头部
}