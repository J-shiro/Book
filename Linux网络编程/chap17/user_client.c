#include "nf_sockopte.h"
#include <arpa/inet.h>
#include <errno.h>
#include <linux/netlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define NETLINK_NF 31    // Netlink 协议号，与内核模块保持一致
#define MAX_PAYLOAD 1024 // Netlink 数据包最大长度

// 创建 Netlink 套接字并发送命令
int send_netlink_msg(int cmd, struct band_status *status) {
  int sock_fd;
  struct sockaddr_nl sa;
  struct nlmsghdr *nlh;
  struct iovec iov;
  struct msghdr msg;
  int ret;

  // 创建 Netlink 套接字
  sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_NF);
  if (sock_fd < 0) {
    perror("socket() create fail");
    return -1;
  }

  memset(&sa, 0, sizeof(sa));
  sa.nl_family = AF_NETLINK;
  sa.nl_pid = 0;    // 发送到内核
  sa.nl_groups = 0; // 消息不属于任何组

  // 分配 Netlink 消息缓冲区
  nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
  if (!nlh) {
    perror("malloc() fail");
    close(sock_fd);
    return -1;
  }

  memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
  nlh->nlmsg_len = NLMSG_SPACE(sizeof(struct band_status));
  nlh->nlmsg_pid = getpid();
  nlh->nlmsg_flags = 0;
  nlh->nlmsg_type = cmd; // 设定 Netlink 命令类型

  // 将 status 数据复制到消息数据区域中，NLMSG_DATA(nlh)获取消息的数据部分的指针
  memcpy(NLMSG_DATA(nlh), status, sizeof(struct band_status));

  iov.iov_base = (void *)nlh;
  iov.iov_len = nlh->nlmsg_len;

  memset(&msg, 0, sizeof(msg));
  msg.msg_name = (void *)&sa;
  msg.msg_namelen = sizeof(sa);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  // 发送 Netlink 消息
  ret = sendmsg(sock_fd, &msg, 0);
  if (ret < 0) {
    perror("sendmsg() fail");
    free(nlh);
    close(sock_fd);
    return -1;
  }

  printf("successfully send Netlink message\n");

  free(nlh);
  close(sock_fd);
  return 0;
}

// 屏蔽 IP 地址
void block_ip(const char *ip) {
  struct band_status status;
  memset(&status, 0, sizeof(status));

  status.band_ip = inet_addr(ip);
  if (send_netlink_msg(SOE_BANDIP, &status) == 0) {
    printf("successfully shield IP: %s\n", ip);
  } else {
    fprintf(stderr, "sheild IP fail: %s\n", ip);
  }
}

// 屏蔽端口
void block_port(int port, int protocol) {
  struct band_status status;
  memset(&status, 0, sizeof(status));

  status.band_port.protocol = protocol;
  status.band_port.port = htons(port);

  if (send_netlink_msg(SOE_BANDPORT, &status) == 0) {
    printf("successfully shield port: %d, protocol: %s\n", port,
           (protocol == IPPROTO_TCP) ? "TCP" : "UDP");
  } else {
    fprintf(stderr, "shield port fail: %d\n", port);
  }
}

// 屏蔽 Ping
void block_ping(int disable) {
  struct band_status status;
  memset(&status, 0, sizeof(status));

  status.band_ping = disable;

  if (send_netlink_msg(SOE_BANDPING, &status) == 0) {
    printf("Ping echo is %s\n", disable ? "banned" : "approved");
  } else {
    fprintf(stderr, "change Ping echo fail\n");
  }
}

// 显示菜单
void show_menu() {
  printf("\n=== hook_demo user control ===\n");
  printf("1. shield IP\n");
  printf("2. shield PORT\n");
  printf("3. ban Ping\n");
  printf("4. exit\n");
  printf("choice: ");
}

int main() {
  int choice;
  char ip[16];
  int port, protocol;

  while (1) {
    show_menu();
    scanf("%d", &choice);

    switch (choice) {
    case 1:
      printf("please input the IP address: ");
      scanf("%s", ip);
      block_ip(ip);
      break;

    case 2:
      printf("please input the port: ");
      scanf("%d", &port);
      printf("choose protocol (1. TCP, 2. UDP): ");
      scanf("%d", &protocol);
      if (protocol == 1) {
        block_port(port, IPPROTO_TCP);
      } else if (protocol == 2) {
        block_port(port, IPPROTO_UDP);
      } else {
        printf("invalid protocol type\n");
      }
      break;

    case 3:
      printf("whether to ban Ping? (1: ban, 0: approve): ");
      scanf("%d", &protocol);
      block_ping(protocol);
      break;

    case 4:
      printf("exit program\n");
      exit(0);
      break;

    default:
      printf("invalid choice\n");
    }
  }

  return 0;
}
