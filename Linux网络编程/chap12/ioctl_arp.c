#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <string.h>
#include <unistd.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>

// 获取主机 IP 地址对应硬件地址

int main(int argc, char *argv[])
{
    int s;
    struct arpreq arpreq;
    struct sockaddr_in *addr = (struct sockaddr_in *)&arpreq.arp_pa;
    unsigned char *hw;
    int err = -1;
    if (argc < 2)
    {
        printf("wrong usage format, usage:\n ./a.out ip(./a.out 127.0.0.1)\n");
        return -1;
    }

    // 建立一个数据报套接字
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
    {
        printf("socket() error\n");
    }

    // 填充 arpreq 的成员 arp_pa
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(argv[1]);
    if (addr->sin_addr.s_addr == INADDR_NONE)
    {
        printf("IP address format wrong\n");
    }

    // 网络接口为 eth0
    memset(arpreq.arp_dev, 0, sizeof(arpreq.arp_dev));
    strcpy(arpreq.arp_dev, "eth0");
    err = ioctl(s, SIOCGARP, &arpreq);
    // 会获取 arp 表中数据, 需要先 ping 通目标 IP
    if (err < 0)
    {
        printf("IOCTL error\n");
        return -1;
    }
    else
    {
        // 成功
        hw = (unsigned char *)&arpreq.arp_ha.sa_data;                                        // 硬件地址
        printf("%s: ", argv[1]);                                                             // 打印 IP
        printf("%02x:%02x:%02x:%02x:%02x:%02x\n", hw[0], hw[1], hw[2], hw[3], hw[4], hw[5]); // 打印硬件地址
    }

    close(s);
    return 0;
}
