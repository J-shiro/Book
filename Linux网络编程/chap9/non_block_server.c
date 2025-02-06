#include<sys/socket.h>
#include<sys/types.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include <sys/fcntl.h>
#include<string.h>
#include<errno.h>

#define PORT 9999
#define BACKLOG 4

// 使用非阻塞方式轮询等待客户端到来，需设置NON_BLOCK方式
// 轮询方式accept 和 recv，HELLO-OK，SHUTDOWN-BYE
int main(int argc, char *argv[]){
    char buffer[1024];
    struct sockaddr_in local, client;
    socklen_t len = sizeof(client);
    int s_s = -1, s_c = -1;

    // 初始化地址结构
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(PORT);

    // 建立套接字描述符
    if((s_s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置非阻塞方式
    fcntl(s_s, F_SETFL, O_NONBLOCK);

    if(bind(s_s, (struct sockaddr *)&local, sizeof(local)) < 0){
        perror("bind failed");
        close(s_s);
        exit(EXIT_FAILURE);
    }
    // 侦听
    if(listen(s_s, BACKLOG) < 0){
        perror("listen failed");
        close(s_s);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);
    
    for(;;){
        // 轮询等待客户端连接
        while((s_c = accept(s_s, (struct sockaddr *)&client, &len)) < 0){
            // EAGAIN:资源暂时不可用，程序应稍后重试
            // EWOULDBLOCK:导致阻塞
            if(errno != EAGAIN && errno != EWOULDBLOCK){
                perror("accept failed");
                close(s_s);
                exit(EXIT_FAILURE);
            }
            usleep(500000); // 等待500ms再尝试
        }

        printf("Client connected: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        // 轮询接收数据
        while(1){
            memset(buffer, 0, sizeof(buffer)); // 清空buffer
            ssize_t bytes_received = recv(s_c, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);

            if(bytes_received < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    usleep(500000); // 等待 500 ms再尝试
                    continue;
                }else{
                    perror("recv failed");
                    close(s_c);
                    break;
                }
            }else if(bytes_received == 0){
                printf("Client disconnected\n");
                close(s_c);
                break;
            }
            buffer[bytes_received - 1] = '\0'; // 确保字符串结尾
            printf("Received: %s, size: %zd\n", buffer, bytes_received);

            if(strcmp(buffer, "HELLO") == 0){
                send(s_c, "OK\n", 2, 0);
                // close(s_c);
                continue;
            }

            if(strcmp(buffer, "SHUTDOWN") == 0){
                send(s_c, "BYE\n", 3, 0);
                close(s_c);
                goto shutdown;
            }

        }
    }
shutdown:
    close(s_s);
    printf("Server shutting down...\n");
    return 0;
}