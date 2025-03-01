#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <error.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <net/if_arp.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <sys/ioctl.h>

int main(int argc, char *argv[])
{
    int s;        // 套接字描述符
    int err = -1; // 错误值

    s = socket(AF_INET, SOCK_DGRAM, 0); // 建立数据报套接字
    if (s < 0)
    {
        printf("socket() error\n");
        return -1;
    }

    // 获取网络接口名称
    struct ifreq ifr;
    ifr.ifr_ifindex = 2; // 获取第 2 个网络接口名称
    err = ioctl(s, SIOCGIFNAME, &ifr);
    if (err)
    {
        printf("SIOCGIFNAME Error\n");
    }
    else
    {
        printf("the %dnd interface is: %s\n", ifr.ifr_ifindex, ifr.ifr_name);
    }

    // 获取网络接口配置参数 查询网卡 eth0 情况
    memcpy(ifr.ifr_name, "eth0", 5);
    err = ioctl(s, SIOCGIFFLAGS, &ifr);
    if (!err)
    {
        printf("SIOCGIFFLAGS: %d\n", ifr.ifr_flags);
    }

    // 获取 METRIC 值
    err = ioctl(s, SIOCGIFMETRIC, &ifr);
    if (!err)
    {
        printf("SIOCGIFMETRIC: %d\n", ifr.ifr_metric);
    }

    // 获取 MTU 和 MAC
    err = ioctl(s, SIOCGIFMTU, &ifr);
    if (!err)
    {
        printf("SIOCGIFMTU: %d\n", ifr.ifr_mtu);
    }
    err = ioctl(s, SIOCGIFHWADDR, &ifr);
    if (!err)
    {
        unsigned char *hw = ifr.ifr_hwaddr.sa_data;
        printf("SIOCGIFHWADDR: %02x:%02x:%02x:%02x:%02x:%02x\n", hw[0], hw[1], hw[2], hw[3], hw[4], hw[5]);
    }

    // 获取网卡映射参数
    err = ioctl(s, SIOCGIFMAP, &ifr);
    if (!err)
    {
        printf("SIOCGIFMAP, mem_start:%ld, mem_end:%ld, base_addr:%d, dma:%d, port:%d\n",
               ifr.ifr_map.mem_start, // 开始地址
               ifr.ifr_map.mem_end,   // 结束地址
               ifr.ifr_map.base_addr, // 基地址
                                      //    ifr.ifr_map.irq,       // 中断
               ifr.ifr_map.dma,       // 直接访问内存
               ifr.ifr_map.port);     // 端口
    }

    // 获取网卡序号
    err = ioctl(s, SIOCGIFINDEX, &ifr);
    if (!err)
    {
        printf("SIOCGIFINDEX: %d\n", ifr.ifr_ifindex);
    }

    // 获取发送队列长度
    err = ioctl(s, SIOCGIFTXQLEN, &ifr);
    if (!err)
    {
        printf("SIOCGIFTXQLEN: %d\n", ifr.ifr_qlen);
    }

    // 获取网络接口 IP 地址
    struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr; // 方便操作设置指向 sockaddr_in 的指针
    char ip[16];                                                   // IP 地址字符串
    memset(ip, 0, 16);
    memcpy(ifr.ifr_name, "eth0", 5);

    // 查询本地 IP 地址
    err = ioctl(s, SIOCGIFADDR, &ifr);
    if (!err)
    {
        // 将整型转化为点分四段的字符串
        inet_ntop(AF_INET, &sin->sin_addr.s_addr, ip, 16);
        printf("SIOCGIFADDR: %s\n", ip);
    }

    // 查询广播 IP 地址
    err = ioctl(s, SIOCGIFBRDADDR, &ifr);
    if (!err)
    {
        inet_ntop(AF_INET, &sin->sin_addr.s_addr, ip, 16);
        printf("SIOCGIFBRDADDR: %s\n", ip);
    }

    // 查询目的 IP 地址
    err = ioctl(s, SIOCGIFDSTADDR, &ifr);
    if (!err)
    {
        inet_ntop(AF_INET, &sin->sin_addr.s_addr, ip, 16);
        printf("SIOCGIFDSTADDR: %s\n", ip);
    }

    // 查询子网掩码
    err = ioctl(s, SIOCGIFNETMASK, &ifr);
    if (!err)
    {
        inet_ntop(AF_INET, &sin->sin_addr.s_addr, ip, 16);
        printf("SIOCGIFNETMASK: %s\n", ip);
    }

    // 设置网络接口 IP 地址
    // memset(&ifr, 0, sizeof(ifr)); // 初始化
    // memcpy(ifr.ifr_name, "eth0", 5);
    // inet_pton(AF_INET, "192.168.1.175", &sin->sin_addr.s_addr); // 字符串转换为网络字节序的整型
    // sin->sin_family = AF_INET;
    // err = ioctl(s, SIOCSIFADDR, &ifr); // 设置本机 IP 地址请求命令
    // if (err)
    // {
    //     printf("SIOCSIFADDR error\n");
    // }
    // else
    // {
    //     printf("check IP:  ");
    //     memset(&ifr, 0, sizeof(ifr));
    //     memcpy(ifr.ifr_name, "eth0", 5);
    //     ioctl(s, SIOCGIFADDR, &ifr); // 读取
    //     inet_ntop(AF_INET, &sin->sin_addr.s_addr, ip, 16);
    //     printf("%s\n", ip);
    // }

    close(s);
    return 0;
}