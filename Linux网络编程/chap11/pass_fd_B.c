#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

// 进程B获得进程A中发送过来的消息，并从中获取文件描述符，根据文件描述符直接从文件中读取数据，并将其在标准输出打印出来

// 从fd接收消息，并将文件描述符放在指针recvfd中
ssize_t recv_fd(int fd, void *data, size_t bytes, int *recvfd)
{
    struct msghdr msghdr_recv; // 接收消息结构
    struct iovec iov[1];       // 接收消息的向量
    size_t n;
    int newfd;

    union
    {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    } control_un;

    struct cmsghdr *pcmsghdr; // 头部控制消息结构

    msghdr_recv.msg_control = control_un.control;
    msghdr_recv.msg_controllen = sizeof(control_un.control);

    msghdr_recv.msg_name = NULL;
    msghdr_recv.msg_namelen = 0;

    iov[0].iov_base = data;
    iov[0].iov_len = bytes;

    msghdr_recv.msg_iov = iov;
    msghdr_recv.msg_iovlen = 1;
    if ((n = recvmsg(fd, &msghdr_recv, 0)) <= 0) // 接收消息
    {
        perror("recvmsg failed\n");
        return n;
    }
    if ((pcmsghdr = CMSG_FIRSTHDR(&msghdr_recv)) != NULL &&
        pcmsghdr->cmsg_len == CMSG_LEN(sizeof(int)))
    {
        if (pcmsghdr->cmsg_level != SOL_SOCKET)
            printf("control level != SOL_SOCKET\n");
        if (pcmsghdr->cmsg_type != SCM_RIGHTS)
            printf("control type != SCM_RIGHTS\n");

        *recvfd = *((int *)CMSG_DATA(pcmsghdr)); // 获得文件的文件描述符
    }
    else
        *recvfd = -1;

    return n;
}

int my_open(const char *pathname, int mode)
{
    int fd, sockfd[2], status;
    pid_t childpid;
    char c, argsockfd[10], argmode[10];

    socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd); // sockfd用于存储2个将要通信的fd, 建立socket
    if (((childpid = fork()) == 0))
    {                                                            // 子进程
        close(sockfd[0]);                                        // 关闭 sockfd[0]
        snprintf(argsockfd, sizeof(argsockfd), "%d", sockfd[1]); // socket描述符 向argsockfd写入sockfd[1]
        snprintf(argmode, sizeof(argmode), "%d", mode);          // 打开文件方式 向argmode写入mode

        execl("./A", "A", argsockfd, pathname, argmode, (char *)NULL); // 执行进程A argv0, argv1, argv2, argv3

        // execl 失败才会执行下面的代码
        printf("execl error\n");
        _exit(127); // 终止子进程，避免执行后续代码
    }
    // 父进程
    close(sockfd[1]);
    // 等待子进程结束
    waitpid(childpid, &status, 0);

    if (WIFEXITED(status))
    {
        if (WEXITSTATUS(status) == 0) // 子进程正常结束
        {
            recv_fd(sockfd[0], &c, 1, &fd);
        }
        else
        {
            errno = WEXITSTATUS(status);
            fd = -1;
        }
    }
    else
    {
        printf("child did not terminate\n");
        fd = -1;
    }
    close(sockfd[0]);
    return fd;
}

#define BUFFSIZE 256
int main(int argc, char *argv[])
{
    int fd, n;
    char buff[BUFFSIZE]; // 接收缓冲区

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    if ((fd = my_open(argv[1], O_RDONLY)) < 0)
        printf("can't open %s\n", argv[1]);

    while ((n = read(fd, buff, BUFFSIZE)) > 0) // 读取数据
        write(1, buff, n);                     // 写入标准输出

    return (0);
}