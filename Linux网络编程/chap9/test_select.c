#include<sys/time.h>
#include<sys/types.h>
#include<signal.h>
#include<sys/select.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define STDIN_FD 0  // 标准输入的文件描述符

// select 在多个文件描述符fd上进行I/O多路复用，等待其中任意一个变为可读、可写或发生异常。
int main(void){
    fd_set rd; // 读文件集合
    struct timeval tv; // 时间间隔
    int err;

    // 清空并添加标准输入，监视标准输入是否可以读数据
    FD_ZERO(&rd);
    FD_SET(STDIN_FD, &rd);

    tv.tv_sec = 5; // 设置5s的超时时间
    tv.tv_usec = 0;

    printf("Waiting for input (5 seconds timeout)...\n");

    err = select(STDIN_FD+1, &rd, NULL, NULL, &tv); // 超时轮询方式查看文件读写错误可操作性 监视标准输入

    if(err == -1){
        perror("select error");
        exit(EXIT_FAILURE);
    }
    else if(err)  // 标准输入有数据输入，可读
        printf("Data is available now.\n");
        if(FD_ISSET(STDIN_FD, &rd)){
            char buffer[100];
            read(STDIN_FD, buffer, sizeof(buffer));
            printf("Received input: %s", buffer);
        }
    else
        printf("No data within five seconds. Timeout! \n"); // 超时，无数据到达
    
    return 0;
}