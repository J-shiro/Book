#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<string.h>
#include<stdio.h>
#include<errno.h>
#include<unistd.h>
#include<stdlib.h>
// 进程A根据用户输入的文件名打开一个文件，将文件描述符打包到消息结构中，发送给进程B

// 向文件描述符fd发送消息，讲sendfd打包到消息体中
ssize_t send_fd(int fd, void * data, size_t bytes, int sendfd){
    struct msghdr msghdr_send; // 发送消息结构
    struct iovec iov[1]; // 向量
    size_t n; // 大小
    int newfd; // 文件描述符
    // 方便操作msg的结构
    union{
        struct cmsghdr cm; // control msg 结构
        char control[CMSG_SPACE(sizeof(int))]; // 字符指针，方便控制，CMSG_SPACE宏计算存储控制消息所需的空间大小
    }control_un;

    struct cmsghdr * pcmsghdr = NULL; // 控制头部的指针
    msghdr_send.msg_control = control_un.control; // 控制消息
    msghdr_send.msg_controllen = sizeof(control_un.control); // 长度

    pcmsghdr = CMSG_FIRSTHDR(&msghdr_send); // 取得第一个消息头
    pcmsghdr->cmsg_len = CMSG_LEN(sizeof(int)); // 获得长度，发送文件描述符，为int类型
    pcmsghdr->cmsg_level = SOL_SOCKET; // 用于控制消息
    pcmsghdr->cmsg_type = SCM_RIGHTS; 
    *((int *)CMSG_DATA(pcmsghdr)) = sendfd; // socket值

    msghdr_send.msg_name = NULL; // 名称
    msghdr_send.msg_namelen = 0; // 名称长度

    iov[0].iov_base = data; // 向量指针指向数据
    iov[0].iov_len = bytes; // 数据长度

    msghdr_send.msg_iov = iov;
    msghdr_send.msg_iovlen = 1;

    return (sendmsg(fd, &msghdr_send, 0));
}

int main(int argc, char * argv[]){
    int fd;
    ssize_t n;

    if(argc != 4)
        printf("socketpair error\n");
    if((fd = open(argv[2], atoi(argv[3]))) < 0) // 打开输入的文件名称
        return(0);
    // 将打开的文件描述符fd传递给输入的socket文件描述符
    if((n = send_fd(atoi(argv[1]), "", 1, fd)) < 0) // 发送文件描述符
        return(0);
}