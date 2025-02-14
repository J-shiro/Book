#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<string.h>
#include<signal.h>
#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include<unistd.h>

// 原始套接字

// 错误处理函数
static void display_err(const char * on_what){
    perror(on_what);
    exit(1);
}

int main(int argc, char * argv[]){
    int error;
    int sock_UNIX; // 套接字
    struct sockaddr_un addr_UNIX; // AF_UNIX族地址
    socklen_t len_UNIX; // AF_UNIX族地址长度
    const char path[] = "/Users/jshiro/Usually/github_local/Book/Linux网络编程/chap11/demo";

    // 建立套接字
    sock_UNIX = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sock_UNIX == -1){
        display_err("socket() error");
    }

    // 由于之前使用了path路径用于其他用途，需取消之前绑定
    unlink(path);

    // 填充地址结构
    memset(&addr_UNIX, 0, sizeof(addr_UNIX));

    addr_UNIX.sun_family = AF_LOCAL;
    strcpy(addr_UNIX.sun_path, path);
    len_UNIX = sizeof(struct sockaddr_un); // SUN_LEN(addr_un)也可

    // 绑定地址到套接字
    error = bind(sock_UNIX, (struct sockaddr *)&addr_UNIX, len_UNIX);
    if(error == -1) display_err("bind() error");

    // 关闭socket
    close(sock_UNIX);
    unlink(path);
    return 0;
}