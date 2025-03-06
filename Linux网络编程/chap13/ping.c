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

// 保存已经发送包的状态值
typedef struct pingm_packet
{
    struct timeval tv_begin; // 发送时间
    struct timeval tv_end;   // 接受时间
    short seq;               // 序列号
    int flag;                // 1 表示已经发送但未接收到回应包 0 表示接受到回应包
} pingm_packet;

static pingm_packet pingpacket[128];

static pingm_packet *icmp_find_packet(int seq);
static unsigned short icmp_cksum(unsigned char *data, int len);
static void icmp_pack(struct icmphdr *icmph, int seq, struct timeval *tv, int length);
static struct timeval icmp_tvsub(struct timeval end, struct timeval begin);
static int icmp_unpack(char *buf, int len);
static void *icmp_send(void *argv);
static void *icmp_recv(void *argv);
static void icmp_statistics(void);
static void icmp_sigint(int signo);
static void icmp_usage();
#define K 1024
#define BUFFERSIZE 72 // 发送缓冲区大小

static unsigned char send_buff[BUFFERSIZE];
static unsigned char recv_buff[2 * K]; // 防止接收溢出
static struct sockaddr_in dest;        // 目的地址
static int rawsock = 0;                // socket 描述符
static pid_t pid = 0;                  // 进程 PID
static int alive = 0;
static short packet_send = 0; // 已经发送的数据包
static short packet_recv = 0; // 已经接收的数据包
static char dest_str[80];     // 目的主机字符串
static struct timeval tv_begin, tv_end, tv_interval;

static void icmp_usage()
{
    printf("ping aaa.bbb.ccc.ddd\n");
}

// 终端信号处理函数 SIGINT ctrl+C
static void icmp_sigint(int signo)
{
    alive = 0;                                  // 告诉接收和发送线程结束
    gettimeofday(&tv_end, NULL);                // 读取程序结束时间
    tv_interval = icmp_tvsub(tv_end, tv_begin); // 计算总时间

    return;
}

// CRC16 校验和计算 16位
static unsigned short icmp_cksum(unsigned char *data, int len)
{
    int sum = 0;          // 计算结果
    int odd = len & 0x01; // 是否为奇数
    unsigned short *value = (unsigned short *)data;

    // 每次处理 2 字节: 16 位 1111 1111 1111 1110
    while (len & 0xfffe)
    {
        sum += *(unsigned short *)data;
        data += 2;
        len -= 2;
    }
    if (odd)
    {
        // ICMP 报头为奇数个字节，会剩下最后一字节
        unsigned short tmp = ((*data) << 8) & 0xff00; // 高 8 位, 低 8 位填充为 0
        sum += tmp;
    }

    sum = (sum >> 16) + (sum & 0xffff); // 高低位相加
    sum += (sum >> 16);                 // 将溢出位加入

    return ~sum; // 返回值取反
}

// 设置 ICMP 报头
static void icmp_pack(struct icmphdr *icmph, int seq, struct timeval *tv, int length)
{
    unsigned int i = 0;
    // 设置报头
    icmph->type = ICMP_ECHO; // ICMP 回显请求
    icmph->code = 0;
    icmph->checksum = 0;
    icmph->un.echo.sequence = seq;
    icmph->un.echo.id = pid & 0xffff;
    icmph->checksum = icmp_cksum((unsigned char *)icmph, length);
}

// 查找合适包位置, seq = -1 表示查找空包, 其他值表示查找seq对应包
static pingm_packet *icmp_find_packet(int seq)
{
    int i = 0;
    pingm_packet *found = NULL;

    // 查找包位置
    if (seq == -1)
    {
        // 查找空包位置
        for (i = 0; i < 128; i++)
        {
            if (pingpacket[i].flag == 0)
            {
                found = &pingpacket[i];
                break;
            }
        }
    }
    else if (seq >= 0)
    {
        for (i = 0; i < 128; i++)
        {
            if (pingpacket[i].seq == seq)
            {
                found = &pingpacket[i];
                break;
            }
        }
    }
    return found;
}

// 计算时间差
static struct timeval icmp_tvsub(struct timeval end, struct timeval begin)
{
    struct timeval tv;
    // 计算差值
    tv.tv_sec = end.tv_sec - begin.tv_sec;
    tv.tv_usec = end.tv_usec - begin.tv_usec;

    // 若接收时间 usec 值小于发送时 usec 值，从 usec 域借位
    if (tv.tv_usec < 0)
    {
        tv.tv_sec--;
        tv.tv_usec += 1000000;
    }
    return tv;
}

// 剥离 IP 头部，分析 ICMP 头部值
static int icmp_unpack(char *buf, int len)
{
    int i, iphdrlen;
    struct iphdr *ip = NULL;
    struct icmphdr *icmp = NULL;
    int rtt;

    ip = (struct iphdr *)buf;                  // IP 头部
    iphdrlen = ip->ihl * 4;                    // IP头部长度
    icmp = (struct icmphdr *)(buf + iphdrlen); // ICMP 段的地址
    len -= iphdrlen;

    if (len < 8)
    {
        printf("ICMP packets\'s length is less than 8\n");
        return -1;
    }

    if ((icmp->type == ICMP_ECHOREPLY) && (icmp->un.echo.id == pid))
    {
        // 类型为 ICMP_ECHOREPLY，且为本进程 PID
        struct timeval tv_internal, tv_recv, tv_send;

        // 在发送表格中查找已经发送的包, 按照 seq
        pingm_packet *packet = icmp_find_packet(icmp->un.echo.sequence);
        if (packet == NULL)
            return -1;
        packet->flag = 0;           // 取消标志
        tv_send = packet->tv_begin; // 获取本包的发送时间

        gettimeofday(&tv_recv, NULL); // 读取此时间，计算时间差
        tv_internal = icmp_tvsub(tv_recv, tv_send);
        rtt = tv_internal.tv_sec * 1000 + tv_internal.tv_usec / 1000;

        // 打印结果
        printf("%d byte from %s: icmp_seq=%u ttl=%d rtt=%d ms\n",
               len,
               inet_ntoa(*(struct in_addr *)&ip->saddr),
               icmp->un.echo.sequence,
               ip->ttl,
               rtt);

        packet_recv++; // 接收包数量加 1
    }
    else
    {
        return -1;
    }
}

// 发送报文
static void *icmp_send(void *argv)
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    // 保存程序开始发送数据的时间
    gettimeofday(&tv_begin, NULL);
    while (alive)
    {
        int size = 0;
        struct timeval tv;
        gettimeofday(&tv, NULL); // 当前包的发送时间

        icmp_pack((struct icmphdr *)send_buff, packet_send, &tv, 64);

        // 打包数据发送给目标地址
        size = sendto(rawsock, send_buff, 64, 0, (struct sockaddr *)&dest, sizeof(dest));
        if (size < 0)
        {
            perror("sendto");
            continue;
        }
        else
        {
            // 在发送包状态数组中找一个空闲位置
            pingm_packet *packet = icmp_find_packet(-1);
            if (packet)
            {
                packet->seq = packet_send;             // 设置seq
                packet->flag = 1;                      // 已经使用
                gettimeofday(&packet->tv_begin, NULL); // 发送时间
                packet_send++;                         // 计数增加
            }
        }
        // 每隔 1 s发送 ICMP 回显请求包
        sleep(1);
    }
}

// 接收报文
static void *icmp_recv(void *argv)
{
    // 轮询等待时间
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 200;
    fd_set readfd;

    // 当没有信号发出一直接收数据
    while (alive)
    {
        int ret = 0;
        FD_ZERO(&readfd);
        FD_SET(rawsock, &readfd);
        ret = select(rawsock + 1, &readfd, NULL, NULL, &tv);
        switch (ret)
        {
        case -1:
            // 错误发生
            break;
        case 0:
            // 超时
            break;
        default:
        {
            // 收到一个包
            int fromlen = 0;
            struct sockaddr from;
            // 接收数据
            int size = recv(rawsock, recv_buff, sizeof(recv_buff), 0);
            if (errno == EINTR) // Interrupted System Call
            {
                perror("recvfrom");
                continue;
            }
            // 解包
            ret = icmp_unpack(recv_buff, size);
            if (ret == -1)
            {
                continue;
            }
        }
        break;
        }
    }
}

// 统计数据结果函数
static void icmp_statistics(void)
{
    long time = (tv_interval.tv_sec * 1000) + (tv_interval.tv_usec / 1000);
    printf("\n--- %s ping statistics ---\n", dest_str); // 目的 IP 地址
    printf("%d packets transmitted, %d received, %d%c packet loss, time %ldms\n",
           packet_send,
           packet_recv,
           (packet_send - packet_recv) * 100 / packet_send,
           '%',
           time);
}

// 两个线程，一个线程发送请求，一个线程接收主机响应，alive为0时退出
int main(int argc, char *argv[])
{
    struct hostent *host = NULL;
    struct protoent *protocol = NULL;
    char protoname[] = "icmp";
    unsigned long inaddr = 1;
    int size = 128 * K;

    if (argc < 2)
    {
        icmp_usage();
        return -1;
    }

    protocol = getprotobyname(protoname);
    if (protocol == NULL)
    {
        perror("getprotobyname()");
        return -1;
    }

    memcpy(dest_str, argv[1], strlen(argv[1]) + 1);
    memset(pingpacket, 0, sizeof(pingm_packet) * 128);
    rawsock = socket(AF_INET, SOCK_RAW, protocol->p_proto);
    if (rawsock < 0)
    {
        perror("socket");
        return -1;
    }

    pid = getuid();

    setsockopt(rawsock, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    inaddr = inet_addr(argv[1]);
    if (inaddr == INADDR_NONE)
    {
        host = gethostbyname(argv[1]);
        if (host == NULL)
        {
            perror("gethostbyname");
            return -1;
        }

        memcpy((char *)&dest.sin_addr, host->h_addr_list[0], host->h_length);
    }
    else
    {
        memcpy((char *)&dest.sin_addr, &inaddr, sizeof(inaddr));
    }

    inaddr = dest.sin_addr.s_addr;
    printf("PING %s (%ld.%ld.%ld.%ld) 56(84) bytes of data.\n",
           dest_str,
           (inaddr & 0x000000FF) >> 0,
           (inaddr & 0x0000FF00) >> 8,
           (inaddr & 0x00FF0000) >> 16,
           (inaddr & 0xFF000000) >> 24);
    signal(SIGINT, icmp_sigint);

    alive = 1;
    pthread_t send_id, recv_id;
    int err = 0;
    err = pthread_create(&send_id, NULL, icmp_send, NULL);
    if (err < 0)
    {
        return -1;
    }
    err = pthread_create(&recv_id, NULL, icmp_recv, NULL);
    if (err < 0)
    {
        return -1;
    }

    pthread_join(send_id, NULL);
    pthread_join(recv_id, NULL);

    close(rawsock);
    icmp_statistics();
    return 0;
}