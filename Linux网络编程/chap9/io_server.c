#include<stdio.h>
#include<sys/uio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<signal.h>

extern void sig_process(int signo);
#define PORT 8888 // 侦听端口地址
#define BACKLOG 2 // 侦听队列长度

static struct iovec * vs = NULL;

void sig_process(int signo){
    printf("Catch a exit signal\n");

    free(vs);
    _exit(0);
}

void sig_pipe(int sign){
    printf("Catch a SIGPIPE signal\n");
    
    free(vs);
    _exit(0);
}

// recv 和 send 函数实现
// void process_conn_server(int s){
//     ssize_t size = 0;
//     char buffer[1024]; // 数据缓冲区

//     for(;;){
//         size = recv(s, buffer, 1024, 0); // 从套接字中读取数据到缓冲区

//         if(size == 0){
//             return;
//         }

//         sprintf(buffer, "%zd bytes altogether\n", size);
//         send(s, buffer, strlen(buffer) + 1, 0); // 发送给客户端
//     }
// }

// readv 和 write 函数实现
// void process_conn_server(int s){
//     char buffer[30] = {0};
//     ssize_t size = 0;

//     struct iovec * v = (struct iovec*)malloc(3*sizeof(struct iovec)); // 申请3个变量
//     if(!v){
//         printf("Not enough memory\n");
//         return;
//     }
//     memset(v, 0, 3 * sizeof(struct iovec));

//     vs = v; // 挂接全局变量，便于释放管理

//     v[0].iov_base = buffer;         // 0 - 9
//     v[1].iov_base = buffer + 10;    // 10 - 19
//     v[2].iov_base = buffer + 20;    // 20 - 29
//     v[0].iov_len = v[1].iov_len = v[2].iov_len = 10;

//     for(;;){
//         size = readv(s, v, 3); // 从套接字读取放到向量缓冲区中

//         if(size == 0) return;

//         // 构建响应字符，为接收到客户端字节的数量，分别放到3个缓冲区中
//         sprintf(v[0].iov_base, "%zd ", size);
//         sprintf(v[1].iov_base, "bytes alt");
//         sprintf(v[2].iov_base, "ogether.     \n");

//         v[0].iov_len = strlen(v[0].iov_base);
//         v[1].iov_len = strlen(v[1].iov_base);
//         v[2].iov_len = strlen(v[2].iov_base);

//         writev(s, v, 3);
//     }
// }

// recvmsg 和 sendmsg函数实现 该情况出现乱码
void process_conn_server(int s){
    char buffer[30];
    ssize_t size = 0;
    struct msghdr msg; // 消息结构

    struct iovec * v = (struct iovec *)malloc(3*sizeof(struct iovec));
    if(!v){
        printf("Not enough memory\n");
        return;
    }

    vs = v;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_iov = v;
    msg.msg_iovlen = 30;
    msg.msg_flags = 0;

    v[0].iov_base = buffer;         // 0 - 9
    v[1].iov_base = buffer + 10;    // 10 - 19
    v[2].iov_base = buffer + 20;    // 20 - 29
    v[0].iov_len = v[1].iov_len = v[2].iov_len = 10;

    for(;;){
        size = recvmsg(s, &msg, 0); // 从套接字中读取数据放到向量缓冲区中
        if(size == 0) return;

        sprintf(v[0].iov_base,  "%zd     ", size);
        sprintf(v[1].iov_base,  "bytes    ");
        sprintf(v[2].iov_base,  "together\n");

        v[0].iov_len = strlen(v[0].iov_base);
        v[1].iov_len = strlen(v[1].iov_base);
        v[2].iov_len = strlen(v[2].iov_base);
        sendmsg(s, &msg, 0); // 发给客户端
    }

}

int main(int argc, char *argv[]){
    int ss, sc;
    struct sockaddr_in server_addr, client_addr;
    int err;
    pid_t pid;

    signal(SIGINT, sig_process);
    signal(SIGPIPE, sig_pipe);

    ss = socket(AF_INET, SOCK_STREAM, 0);
    if(ss < 0){
        printf("socket error\n");
        return -1;
    }

    // 设置服务器地址
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    err = bind(ss, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(err < 0){
        printf("bind error\n");
        return -1;
    }
    err = listen(ss, BACKLOG);
    if(err < 0){
        printf("listen error\n");
        return -1;
    }

    // 主循环过程
    for(;;){
        socklen_t addrlen = sizeof(struct sockaddr);

        sc = accept(ss, (struct sockaddr*)&client_addr, &addrlen);
        if(sc < 0){
            continue;
        }

        // 建立一个新的进程处理到来的连接
        pid = fork();
        if(pid == 0){ // 子进程
            close(ss);  // 子进程关闭服务器侦听
            process_conn_server(sc);
        }else{
            close(sc); // 父进程关闭客户端连接
        }
    }
}