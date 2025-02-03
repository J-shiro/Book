#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#define ADDRLEN 16

int main(int argc, char *argv[]){
    struct in_addr ip, local, network;

    char addr1[] = "192.168.1.1";
    char addr2[] = "255.255.255.255";
    char addr3[] = "192.16.1";
    char addr[ADDRLEN];
    char *str = NULL, *str2 = NULL;

    int err = 0;

    // 测试 inet_aton
    err = inet_aton(addr1, &ip);
    if(err){
        printf("inet_aton: string %s value is : 0x%x\n", addr1, ip.s_addr);
    }else{
        printf("inet_aton: string %s error\n", addr1);
    }

    // 测试 inet_addr 192.168.1.1  255.255.255.255 不可重入
    ip.s_addr = inet_addr(addr1);
    if(ip.s_addr == INADDR_NONE){
        printf("inet_addr: string %s error\n", addr1);
    }else{
        printf("inet_addr: string %s value is : 0x%x\n", addr1, ip.s_addr);
    }

    ip.s_addr = inet_addr(addr2);
    if(ip.s_addr == INADDR_NONE){
        printf("inet_addr: string %s error\n", addr2);
    }else{
        printf("inet_addr: string %s value is : 0x%x\n", addr2, ip.s_addr);
    }

    // 测试 inet_ntoa 不可重入
    ip.s_addr = 192<<24 | 168<<16 | 1<<8 | 1;
    str = inet_ntoa(ip);
    ip.s_addr = 255<<24 | 255<<16 | 255<<8 | 255;
    str2 = inet_ntoa(ip);
    printf("inet_ntoa: ip:0x%x string1 %s, pre is : %s\n", ip.s_addr, str2, str);

    // 测试 inet_lnaof inet_netof
    inet_aton(addr1, &ip);
    local.s_addr = inet_lnaof(ip);
    str = inet_ntoa(local);
    printf("inet_lnaof: string %s ip: 0x%x \n", str, local.s_addr);
    
    network.s_addr = inet_netof(ip);
    printf("inet_netof: value : 0x%x \n", network.s_addr);

    // 测试 inet_pton inet_ntop
    printf("inet_pton inet_ntop test......\n");

    err = inet_pton(AF_INET, addr1, &ip); // 字符串转换为二进制
    if(err){
        printf("inet_pton: ip %s value is : 0x%x\n", addr1, ip.s_addr);
    }

    ip.s_addr = htonl(192<<24 | 168<<16 | 12<<8 | 255);
    str = (char *)inet_ntop(AF_INET, (void *)&ip, (char *)&addr[0], ADDRLEN); // 二进制转换为字符串
    if(str){
        printf("inet_ntop: ip 0x%x string is : %s\n", ip.s_addr, str);
    }
    
    return 0;
}