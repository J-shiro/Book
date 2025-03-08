// 线程模型，线程的 accept 函数中，多线程可以使用此函数处理客户端连接，为防止冲突使用线程互斥锁
// accept 前加锁, 后解锁

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFLEN 1024
#define SERVER_PORT 8888
#define BACKLOG 5
#define CLIENTNUM 2

// 互斥量
pthread_mutex_t ALOCK = PTHREAD_MUTEX_INITIALIZER;

static void *handle_request(void *argv)
{
    int s_s = *((int *)argv);
    int s_c; // 客户端套接字文件描述符
    struct sockaddr_in from;
    socklen_t len = sizeof(from);
    for (;;)
    {
        time_t now;
        char buff[BUFFLEN]; // 收发数据缓冲区
        int n = 0;

        pthread_mutex_lock(&ALOCK); // 进入互斥区
        s_c = accept(s_s, (struct sockaddr *)&from, &len);

        // 接收客户端请求
        pthread_mutex_unlock(&ALOCK); // 离开互斥区

        memset(buff, 0, BUFFLEN);
        n = recv(s_c, buff, BUFFLEN, 0); // 接收发送方数据
        if (n > 0 && !strncmp(buff, "TIME", 4))
        {
            memset(buff, 0, BUFFLEN);
            now = time(NULL);
            sprintf(buff, "%24s\r\n", ctime(&now));
            send(s_c, buff, strlen(buff), 0);
        }
        close(s_c);
    }
    return NULL;
}

static void handle_connect(int s)
{
    int s_s = s;
    pthread_t thread_do[CLIENTNUM]; // 线程 ID
    int i = 0;
    for (i = 0; i < CLIENTNUM; i++)
    {
        // 创建线程
        pthread_create(&thread_do[i],  // 线程ID
                       NULL,           // 属性
                       handle_request, // 线程回调函数
                       (void *)&s_s    // 线程参数
        );
    }

    // 等待线程结束
    for (i = 0; i < CLIENTNUM; i++)
    {
        pthread_join(thread_do[i], NULL);
    }
}

int main(int argc, char *argv[])
{
    int s_s;                  // 服务器套接字文件描述符
    struct sockaddr_in local; // 本地地址

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

    // 处理客户端连接
    handle_connect(s_s);

    close(s_s);
    return 0;
}