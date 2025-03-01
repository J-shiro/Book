#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <arpa/inet.h>
// #include <Linux/arp.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if_arp.h>

/*
    使用 SOCK_PACKET 编写 ARP 请求
    以太网头部 + ARP 请求/应答
    以太网头部：目的硬件地址 源硬件地址 帧类型
    ARP 请求/应答：硬件类型 协议类型 硬件地址长度 协议地址长度 操作码 发送方硬件地址 发送方协议地址 接收方硬件地址 接收方协议地址
*/
#define ETH_ALEN 6         // 6字节MAC地址长度
#define ETH_FRAME_LEN 1514 // 以太帧最大长度
#define ETH_HLEN 14        // 以太网帧头长度

struct arppacket
{
    unsigned short ar_hrd;          // 硬件类型
    unsigned short ar_pro;          // 协议类型
    unsigned char ar_hln;           // 硬件地址长度
    unsigned char ar_pln;           // 协议地址长度
    unsigned short ar_op;           // 操作码
    unsigned char ar_sha[ETH_ALEN]; // 发送方MAC地址
    unsigned char ar_sip[4];        // 发送方IP地址
    unsigned char ar_tha[ETH_ALEN]; // 接收方MAC地址
    unsigned char ar_tip[4];        // 接收方IP地址
};

int main(int argc, char *argv[])
{
    char ef[ETH_FRAME_LEN];  // 以太帧缓冲区
    struct ethhdr *p_ethhdr; // 以太网头部指针

    char eth_dest[ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // 目的MAC地址 1
    char eth_src[ETH_ALEN] = {0x38, 0x68, 0x93, 0x7E, 0xAD, 0xD3};  // 源MAC地址

    int n;
    int fd; // fd 是套接字描述符
    // fd = socket(AF_INET, SOCK_PACKET, htons(0x0003)); // 创建套接字 0x0003 表示截取的数据帧类型不确定，处理所有包
    fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); // 创建套接字

    if (fd < 0)
    {
        perror("socket");
        exit(1);
    }

    struct sockaddr_ll sll; // 套接字地址结构
    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_ARP);      // ARP 协议
    sll.sll_ifindex = if_nametoindex("eth0"); // 网卡接口索引

    if (bind(fd, (struct sockaddr *)&sll, sizeof(sll)) < 0)
    {
        perror("bind");
        close(fd);
        exit(1);
    }

    p_ethhdr = (struct ethhdr *)ef;                // 使 p_ethhdr 指向以太网帧的帧头
    memcpy(p_ethhdr->h_dest, eth_dest, ETH_ALEN);  // 复制目的MAC地址
    memcpy(p_ethhdr->h_source, eth_src, ETH_ALEN); // 复制源MAC地址
    p_ethhdr->h_proto = htons(0x0806);             // 帧类型 以太网

    struct arppacket *p_arp;
    p_arp = (struct arppacket *)(ef + ETH_HLEN); // 定位 ARP 包地址
    p_arp->ar_hrd = htons(0x1);                  // arp 硬件类型 -> 硬件地址为以太网接口
    p_arp->ar_pro = htons(0x0800);               // 协议类型 -> 高层为 IP 协议
    p_arp->ar_hln = 6;                           // 硬件地址长度
    p_arp->ar_pln = 4;                           // 协议地址长度

    p_arp->ar_op = htons(ARPOP_REQUEST); // ARP 请求操作码

    unsigned int sip = inet_addr("192.168.1.152");
    unsigned int tip = inet_addr("192.168.1.1");

    memcpy(p_arp->ar_sha, eth_src, ETH_ALEN); // 发送方 MAC 地址
    // (unsigned int *)p_arp->ar_sip = inet_addr("192.168.1.152"); // 发送方 IP 地址
    memcpy(p_arp->ar_sip, &sip, sizeof(sip)); // 发送方 IP 地址

    memcpy(p_arp->ar_tha, eth_dest, ETH_ALEN); // 接收方 MAC 地址
    // (unsigned int *)p_arp->ar_tip = inet_addr("192.168.1.1"); // 接收方 IP 地址
    memcpy(p_arp->ar_tip, &tip, sizeof(tip)); // 接收方 IP 地址

    // 发送 ARP 请求 8 次， 间隔 1 s
    int i = 0;
    for (i = 0; i < 8; i++)
    {
        n = write(fd, ef, sizeof(struct ethhdr) + sizeof(struct arppacket)); // 发送数据帧
        if (n < 0)
        {
            perror("write");
            close(fd);
            exit(1);
        }
        sleep(1);
    }
    close(fd);
    return 0;
}