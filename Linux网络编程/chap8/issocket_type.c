#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

// 判定套接字文件描述符函数
int issockettype(int fd){
    struct stat st;
    int err = fstat(fd, &st); // 获得文件状态至st

    if((st.st_mode & S_IFMT) == S_IFSOCK){ // 判断是否为套接字
        return 1;
    }else{
        return 0;
    }
}

int main(void){
    int ret = issockettype(0);
    printf("stdin return value %d\n", ret);

    int s = socket(AF_INET, SOCK_STREAM, 0);
    ret = issockettype(s);
    printf("socket return value %d\n", ret);
    
    return 0;
}