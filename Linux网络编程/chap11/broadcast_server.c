#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define IP_FOUND "IP_FOUND"
#define IP_FOUND_ACK "IP_FOUND_ACK"

void HandleIPFOUND(void *arg)
{
#define BUFFER_LEN 32
    int ret = -1;
    SOCKET sock = -1;
    struct sockaddr_in local_addr; // 本地地址
    struct sockaddr_in from_addr;  // 客户端地址
    int from_len;
    int count = -1;
    fd_set readfd;
}