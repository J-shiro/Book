#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/uio.h>
#include<signal.h>

extern void sig_process(int signo);
extern void sig_pipe(int signo);
static struct iovec * vc = NULL;
static int s;

// recv 和 send 函数实现
// void process_conn_client(int s){
//     ssize_t size = 0;
//     char buffer[1024];

//     for(;;){
//         size = read(0, buffer, 1024);

//         if(size > 0){
//             send(s, buffer, size, 0);
//             size = recv(s, buffer, 1024, 0);
//             write(1, buffer, size);
//         }
//     }
// }

// readv 和 write 函数实现
// void process_conn_client(int s){
//     char buffer[30];
//     ssize_t size = 0;

//     struct iovec * v = (struct iovec *)malloc(3 * sizeof(struct iovec));
//     if(!v){
//         printf("Not enough memory\n");
//         return;
//     }

//     memset(v, 0, 3 * sizeof(struct iovec));

//     vc = v; // 挂接全局变量
//     v[0].iov_base = buffer;
//     v[1].iov_base = buffer + 10;
//     v[2].iov_base = buffer + 20;

//     v[0].iov_len = v[1].iov_len = v[2].iov_len = 10;

//     int i = 0;
//     for(;;){
//         size = read(0, v[0].iov_base, 10); // 从标准输入中读取数据放到缓冲区buffer中
//         if(size > 0){
//             v[0].iov_len = size;
//             writev(s, v, 1); // 发送给服务器
//             v[0].iov_len = v[1].iov_len = v[2].iov_len = 10;
//             size = readv(s, v, 3); // 从服务器读取数据
//             for(i = 0; i < 3; i++){
//                 if(v[i].iov_len > 0){
//                     write(1, v[i].iov_base, v[i].iov_len); // 写到标准输出
//                 }
//             }
//         }
//     }
// }

// recvmsg 和 sendmsg函数实现
void process_conn_client(int s){
    char buffer[30];
    ssize_t size = 0;
    struct msghdr msg;

    struct iovec * v = (struct iovec *)malloc(3 * sizeof(struct iovec));
    if(!v){
        printf("Not enough memory\n");
        return;
    }

    vc = v;

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

    int i = 0;
    for(;;){
        size = read(0, v[0].iov_base, 10); // 从标准输入中读取数据到缓冲区buffer中

        if(size > 0){
            v[0].iov_len = size;
            sendmsg(s, &msg, 0); // 发送给服务器
            v[0].iov_len = v[1].iov_len = v[2].iov_len = 10;
            size = recvmsg(s, &msg, 0);
            for(i = 0; i < 3; i++){
                if(v[i].iov_len > 0){
                    write(1, v[i].iov_base, v[i].iov_len); // 写到标准输出
                }
            }
        }
    }
}

void sig_process(int signo) // 客户端信号处理回调函数
{
    printf("Catch a exit signal\n");

    free(vc);
    _exit(0);
}

void sig_pipe(int sign){
    printf("Catch a SIGPIPE signal\n");
    
    free(vc);
    _exit(0);
}

# define PORT 8888

int main(int argc, char *argv[]){
    struct sockaddr_in server_addr;
    int err;

    if(argc == 1){
        printf("Please input server addr\n");
        return 0;
    }
    signal(SIGINT, sig_process);
    signal(SIGPIPE, sig_pipe);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s < 0){
        printf("socket error\n");
        return -1;
    }

    // 设置服务器地址
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // 将用户输入的字符串类型的IP地址转换为整型
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr); 
    // 连接服务器
    connect(s, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    process_conn_client(s);

    close(s);
}