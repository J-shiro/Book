// IO 复用循环服务器模型

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFLEN 1024
#define SERVER_PORT 8888
#define BACKLOG 5
#define CLIENTNUM 1024 // 最大支持客户端数量

// 可连接客户端的文件描述符数组
int connect_host[CLIENTNUM];
int connect_number = 0;

// 客户端连接处理业务
static void *handle_connect(void *argv)
{
    int s_s = *((int *)argv); // 获取服务器侦听套接字文件描述符
    int s_c = -1;             // 连接客户端文件描述符

    struct sockaddr_in from;
    socklen_t len = sizeof(from);

    // 接受客户端连接
    for (;;)
    {
        int i = 0;
        int s_c = accept(s_s, (struct sockaddr *)&from, &len);

        // 接收客户端请求
        printf("a client connect, from: %s\n", inet_ntoa(from.sin_addr));

        // 查找合适位置，将客户端文件描述符放入
        for (i = 0; i < CLIENTNUM; i++)
        {
            if (connect_host[i] == -1)
            {
                // 放入
                connect_host[i] = s_c;

                connect_number++;
                break; // 继续轮询等待客户端连接
            }
        }
    }
}

// 客户端请求处理业务
static void *handle_request(void *argv)
{
    time_t now; // 时间
    // 使用 IO 复用的 select 对多个套接字文件描述符进行一定时间等待

    char buff[BUFFLEN]; // 收发数据缓冲区
    int n = 0;

    int maxfd = -1;         // 最大侦听文件描述符
    fd_set scanfd;          // 侦听描述符集
    struct timeval timeout; // 超时
    timeout.tv_sec = 1;     // 阻塞 1 s 后超时返回
    timeout.tv_usec = 0;

    int i = 0;
    int err = -1;

    for (;;)
    {
        // 最大文件描述符值初始化为-1
        maxfd = -1;

        FD_ZERO(&scanfd); // 清零文件描述符集合
        for (i = 0; i < CLIENTNUM; i++)
        {
            // 将文件描述符放入到集合中
            if (connect_host[i] != -1)
            {
                FD_SET(connect_host[i], &scanfd);
                if (maxfd < connect_host[i])
                {
                    maxfd = connect_host[i];
                }
            }
        }

        // select 等待
        err = select(maxfd + 1, &scanfd, NULL, NULL, &timeout);
        switch (err)
        {
        case 0: // 超时
            break;
        case -1: // 错误发生
            break;
        default: // 有可读套接字文件描述符
            if (connect_number <= 0)
            {
                break;
            }

            for (i = 0; i < CLIENTNUM; i++)
            {
                // 查找激活的文件描述符 测试指定的文件描述符是否在该集合中
                if (FD_ISSET(connect_host[i], &scanfd))
                {
                    memset(buff, 0, BUFFLEN); // 清零
                    n = recv(connect_host[i], buff, BUFFLEN, 0);
                    // 接收发送方数据
                    if (n > 0 && !strncmp(buff, "TIME", 4))
                    {
                        // 判断是否合法接收数据
                        memset(buff, 0, BUFFLEN);
                        now = time(NULL);
                        sprintf(buff, "%24s\r\n", ctime(&now));
                        send(connect_host[i], buff, strlen(buff), 0); // 发送数据
                    }

                    // 更新文件描述符在数组中的值
                    connect_host[i] = -1;
                    connect_number--;

                    close(connect_host[i]);
                }
            }
            break;
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int s_s;                  // 服务器套接字文件描述符
    struct sockaddr_in local; // 本地地址
    int i = 0;
    memset(connect_host, -1, CLIENTNUM);

    // 建立 TCP 套接字
    s_s = socket(AF_INET, SOCK_STREAM, 0);

    // 初始化地址和端口
    memset(&local, 0, sizeof(local)); // 清零
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(SERVER_PORT);

    // 将套接字文件描述符绑定到本地地址和端口
    int err = bind(s_s, (struct sockaddr *)&local, sizeof(local));
    err = listen(s_s, BACKLOG); // 侦听

    pthread_t thread_do[2]; // 线程 ID
    // 创建线程处理客户端连接
    pthread_create(&thread_do[0],  // 线程ID
                   NULL,           // 属性
                   handle_connect, // 线程回调函数
                   (void *)&s_s    // 线程参数
    );

    // 创建线程处理客户端请求
    pthread_create(&thread_do[1],  // 线程ID
                   NULL,           // 属性
                   handle_request, // 线程回调函数
                   NULL            // 线程参数
    );

    for (i = 0; i < 2; i++)
    {
        pthread_join(thread_do[i], NULL);
    }

    close(s_s);
    return 0;
}