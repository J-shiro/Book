#include<sys/time.h>
#include<sys/types.h>
#include<signal.h>
#include<sys/select.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

int child_events = 0; // 记录 SIGCHLD 触发次数

// 信号处理函数
void child_sig_handler(int x){
    child_events++; // 调用次数+1
    signal(SIGCHLD, child_sig_handler); // 重新设定信号回调函数
}

// pselect 中 sigset_t sigmask 允许在 pselect 调用期间屏蔽特定信号，避免被信号中断
int main(int argc, char **argv){
    // 设定的信号掩码sigmask， 原始的信号掩码orig_sigmask
    sigset_t sigmask, orig_sigmask;
    fd_set rd, wr, er;
    struct timespec timeout;
    int nfds  = 1; // 监视标准输入 fd=0
    int err;
    
    timeout.tv_sec = 5;
    timeout.tv_nsec = 0;

    // 阻塞 SIGCHLD， 避免条件竞争
    sigemptyset(&sigmask); // 清空信号
    sigaddset(&sigmask, SIGCHLD); // 将信号加入sigmask

    // 设定信号 SIG_BLOCK的掩码sigmask， 将原始的掩码保存到orig_sigmask
    sigprocmask(SIG_BLOCK, &sigmask, &orig_sigmask);

    // 挂接对信号SIGCHLD的处理函数child_sig_handler
    signal(SIGCHLD, child_sig_handler);

    // fork 子进程， 让其退出产生SIGCHLD
    pid_t pid = fork();
    if(pid == 0){
        sleep(3);
        exit(0); // 触发SIGCHLD
    }

    printf("Waiting for input or SIGCHLD event...\n");
    for(;;){ // 主循环
        
        while(child_events > 0){
            printf("Child process exited, Handling SIGCHLD event.\n");
            child_events--;
        }

        // 初始化 fd_set
        FD_ZERO(&rd);
        FD_SET(STDIN_FILENO, &rd); // 监视标准输入是否可读

        // r = pselect(nfds, &rd, &wr, &er, 0, &orig_sigmask);
        err = pselect(nfds, &rd, NULL, NULL, &timeout, &orig_sigmask);

        if(err == -1){
            perror("pselect error");
            exit(EXIT_FAILURE);
        }
        else if(err)  // 标准输入有数据输入，可读
            printf("Data is available now.\n");
            if(FD_ISSET(STDIN_FILENO, &rd)){
                char buffer[100];
                read(STDIN_FILENO, buffer, sizeof(buffer));
                printf("Received input: %s", buffer);
            }
        else
            printf("No data within five seconds. Timeout! \n"); // 超时，无数据到达

        }

}