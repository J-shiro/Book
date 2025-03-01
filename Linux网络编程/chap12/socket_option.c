#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/in.h>
#include <sys/time.h>

// 定义选项所用通用数据结构体
// 结构保存获取结果, 统一处理 getsockopt() 函数返回值
typedef union optval
{
    int val;               // 整型值
    struct linger linger;  // linger 结构
    struct timeval tv;     // 时间结构
    unsigned char str[16]; // 字符串
} val;

static val optvalue;

// 定义数据结构类型值
typedef enum valtype
{
    VALINT,     // int 类型 0
    VALLINGER,  // linger 1
    VALTIMEVAL, // timeval 2
    VALUCHAR,   // 字符串 3
    VALMAX,     // 错误类型 4
} valtype;

// 保存套接字选项的结构
typedef struct sopts
{
    int level;       // 级别
    int optname;     // 选项值
    char *name;      // 套接字名称
    valtype valtype; // 套接字返回参数类型
} sopts;

sopts sockopts[] = {
    {SOL_SOCKET, SO_BROADCAST, "SO_BROADCAST", VALINT},
    {SOL_SOCKET, SO_DEBUG, "SO_DEBUG", VALINT},
    {SOL_SOCKET, SO_ERROR, "SO_ERROR", VALINT},
    {SOL_SOCKET, SO_KEEPALIVE, "SO_KEEPALIVE", VALINT},
    {SOL_SOCKET, SO_LINGER, "SO_LINGER", VALINT},
    {SOL_SOCKET, SO_RCVTIMEO, "SO_RCVTIMEO", VALTIMEVAL},
    {SOL_SOCKET, SO_SNDTIMEO, "SO_SNDTIMEO", VALTIMEVAL},
    {SOL_SOCKET, SO_TYPE, "SO_TYPE", VALINT},
    {IPPROTO_IP, IP_HDRINCL, "IP_HDRINCL", VALINT},
    {IPPROTO_IP, IP_OPTIONS, "IP_OPTIONS", VALINT},
    {IPPROTO_IP, IP_TTL, "IP_TTL", VALINT},
    {IPPROTO_IP, IP_MULTICAST_TTL, "IP_MULTICAST_TTL", VALUCHAR},
    {IPPROTO_IP, IP_MULTICAST_LOOP, "IP_MULTICAST_LOOP", VALUCHAR},
    {0, 0, NULL, VALMAX}}; // 主程序中判断 valtype 是否为 VALMAX 可知是否到达数组末尾

static void disp_outcome(sopts *sockopt, int len, int err)
{
    if (err == -1)
    {
        printf("optname %s NOT support\n", sockopt->name);
        return;
    }

    switch (sockopt->valtype) // 根据不同类型进行信息打印
    {
    case VALINT: // 整型
        printf("optname %s: default is %d\n", sockopt->name, optvalue.val);
        break;
    case VALLINGER:
        printf("optname %s: default is %d(ON/OFF), %d to linger\n",
               sockopt->name,
               optvalue.linger.l_onoff,   // linger 打开
               optvalue.linger.l_linger); // 延时时间
    case VALTIMEVAL:                      // struct timeval 结构体
        printf("optname %s: default is %.06f\n", sockopt->name,
               ((((double)optvalue.tv.tv_sec * 100000 + (double)optvalue.tv.tv_usec)) / (double)1000000));
        break;
    case VALUCHAR: // 字符串类型
        int i = 0;
        printf("optname %s: default is ", sockopt->name);
        // 选项名称
        for (int i = 0; i < len; i++)
        {
            printf("%02x ", optvalue.str[i]);
        }
        printf("\n");
        break;
    default:
        break;
    }
}

int main()
{
    int err = -1;
    int len = 0;
    int i = 0;
    int s = socket(AF_INET, SOCK_STREAM, 0); // 建立流式套接字
    while (sockopts[i].valtype != VALMAX)
    {
        // 判断是否结尾， 否则轮询执行
        len = sizeof(sopts); // sopts 结构长度
        err = getsockopt(s, sockopts->level, sockopts->optname, &optvalue, &len);

        disp_outcome(&sockopts[i], len, err); // 显示结果
        i++;                                  // 递增
    }
    close(s);
    return 0;
}