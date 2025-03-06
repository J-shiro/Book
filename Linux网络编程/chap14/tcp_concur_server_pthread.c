#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
#include<stdio.h>

#define SERVER_PORT 8888
#define BUFFLEN 1024
#define BACKLOG 5

// 处理客户端请求
static void * handle_request(void *argv){
    int s_c = *(int *)argv;
    time_t now; // 时间
    char buff[BUFFLEN]; // 收发数据缓冲区
    int n = 0;

    memset(buff, 0, BUFFLEN); 
    n = recv(s_c, buff, BUFFLEN, 0); // 接收客户端请求
    if(n > 0 && !strncmp(buff, "TIME", 4)){
        memset(buff, 0, BUFFLEN);
        now = time(NULL);
        sprintf(buff, "%24s\r\n", ctime(&now));// 获取时间
        send(s_c, buff, strlen(buff), 0);
    }

    close(s_c);
    return NULL;
}

// 处理客户端连接
static void handle_connect(int s_s){
    int s_c; // 客户端套接字文件描述符
    struct sockaddr_in from; // 客户端地址
    socklen_t len = sizeof(from);
    pthread_t thread_do;

    while(1){
        s_c = accept(s_s, (struct sockaddr*)&from, &len); // 接受客户端连接
        if(s_c > 0){
            // 创建线程处理连接
            int err = pthread_create(&thread_do,
                                    NULL,
                                    handle_request,
                                    (void *)&s_c);
        }
    }
}

int main(int argc, char *argv[]){
    int s_s; // 服务器套接字文件描述符
    struct sockaddr_in local; // 本地地址

    s_s = socket(AF_INET, SOCK_STREAM, 0); // 创建套接字

    // 初始化地址
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(SERVER_PORT);

    int err = bind(s_s, (struct sockaddr*)&local, sizeof(local)); // 绑定地址
    err = listen(s_s, BACKLOG); // 侦听

    handle_connect(s_s);

    close(s_s);
    return 0;
}