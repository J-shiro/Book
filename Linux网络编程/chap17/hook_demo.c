/*
    动态设置
    - 屏蔽 ping 的回显, 用户 ping 本机时, 本机不进行响应 finish
    - 禁止向某个 IP 发送数据 finish
    - 关闭某端口, 不进行响应
*/

#include "linux/kern_levels.h"
#include "nf_sockopte.h"
#include <linux/capability.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netlink.h>
#include <linux/seq_file_net.h>
#include <linux/skbuff.h>
#include <linux/sockptr.h>
#include <net/tcp.h>

// 版权声明
MODULE_LICENSE("Dual BSD/GPL");

// NF 初始化状态宏
#define NF_SUCCESS 0
#define NF_FAILURE 1

// 选择一个不与系统占用的冲突的协议号
#define NETLINK_NF 31
static struct sock *nl_sk = NULL; // Netlink 套接字 用于接收用户态配置指令

band_status b_status;

// 判断是否禁止 TCP 端口
#define IS_BANDPORT_TCP(status)                                                \
  (status.band_port.port != 0 && status.band_port.protocol == IPPROTO_TCP)
// 判断是否禁止 UDP 端口
#define IS_BANDPORT_UDP(status)                                                \
  (status.band_port.port != 0 && status.band_port.protocol == IPPROTO_UDP)
// 判断是否禁止 ping
#define IS_BANDPING(status) (status.band_ping)
// 判断是否禁止 IP 协议
#define IS_BANDIP(status) (status.band_ip)

// 在 LOCAL_OUT 上挂接钩子, 对从本地发出的数据包进行过滤
static unsigned int nf_hook_out(void *priv, struct sk_buff *skb,
                                const struct nf_hook_state *state) {
  struct iphdr *iph = ip_hdr(skb);

  // 判断是否禁止 IP 协议
  if (IS_BANDIP(b_status)) {
    if (b_status.band_ip == iph->daddr) {
      unsigned int dest_ip = iph->daddr;
      printk(KERN_ALERT "DROP all packets to IP: %d.%d.%d.%d\n",
             (dest_ip & 0x000000ff) >> 0, (dest_ip & 0x0000ff00) >> 8,
             (dest_ip & 0x00ff0000) >> 16, (dest_ip & 0xff000000) >> 24);
      return NF_DROP; // 丢弃该网络报文
    }
  }
  return NF_ACCEPT;
}

// 在 LOCAL_IN 上挂接钩子, 过滤发往本机的数据包
static unsigned int nf_hook_in(void *priv, struct sk_buff *skb,
                               const struct nf_hook_state *state) {
  struct iphdr *iph = ip_hdr(skb);
  unsigned int src_ip = iph->saddr;
  struct tcphdr *tcph = NULL;
  struct udphdr *udph = NULL;

  switch (iph->protocol) {
  // IP 协议类型
  case IPPROTO_TCP:
    if (IS_BANDPORT_TCP(b_status)) {
      tcph = tcp_hdr(skb);
      if (tcph->dest == b_status.band_port.port) {
        printk(KERN_ALERT "DROP packet to Port: %d, protol: tcp\n",
               ntohs(b_status.band_port.port));

        return NF_DROP;
      }
    }
    break;
  case IPPROTO_UDP:
    if (IS_BANDPORT_UDP(b_status)) {
      udph = udp_hdr(skb);
      if (udph->dest == b_status.band_port.port) {
        printk(KERN_ALERT "DROP packet to Port: %d, protol: udp\n",
               ntohs(b_status.band_port.port));
        return NF_DROP;
      }
    }
    break;
  case IPPROTO_ICMP:
    if (IS_BANDPING(b_status)) {
      printk(KERN_ALERT "DROP ICMP packet from %d.%d.%d.%d\n",
             (src_ip & 0x000000ff) >> 0, (src_ip & 0x0000ff00) >> 8,
             (src_ip & 0x00ff0000) >> 16, (src_ip & 0xff000000) >> 24);
      return NF_DROP;
    }
    break;

  default:
    break;
  }
  return NF_ACCEPT;
}
// 初始化钩子 LOCAL_IN 和 LOCAL_OUT
static struct nf_hook_ops nf_hooks[] = {{
                                            .hook = nf_hook_in,
                                            .hooknum = NF_INET_LOCAL_IN,
                                            .pf = PF_INET,
                                            .priority = NF_IP_PRI_FIRST,
                                        },
                                        {
                                            .hook = nf_hook_out,
                                            .hooknum = NF_INET_LOCAL_OUT,
                                            .pf = PF_INET,
                                            .priority = NF_IP_PRI_FIRST,
                                        }

};

// 通过 Netlink 接收 用户空间指令，动态更新 b_status
static void nfnetlink_recv_msg(struct sk_buff *skb) {
  struct nlmsghdr *nlh;
  struct band_status *status;

  if (!skb) {
    printk(KERN_ALERT "skb is Null...");
    return;
  }

  // 解析 Netlink 消息
  nlh = nlmsg_hdr(skb);
  status = (struct band_status *)nlmsg_data(nlh); // 获取数据部分

  // 检查权限
  if (!capable(CAP_NET_ADMIN))
    return;

  switch (nlh->nlmsg_type) {
  case SOE_BANDIP:
    b_status.band_ip = status->band_ip;
    break;
  case SOE_BANDPORT:
    b_status.band_port.protocol = status->band_port.protocol;
    b_status.band_port.port = status->band_port.port;
    break;
  case SOE_BANDPING:
    b_status.band_ping = status->band_ping;
    break;
  default:
    return;
  }
}

// 模块初始化及退出
static int __init hook_module_init(void) {
  struct netlink_kernel_cfg cfg = {
      .input = nfnetlink_recv_msg,
  };

  nl_sk = netlink_kernel_create(&init_net, NETLINK_NF, &cfg);
  if (!nl_sk) {
    printk(KERN_ALERT "Netlink create error\n");
    return -ENOMEM;
  }
  int ret;

  ret = nf_register_net_hooks(&init_net, nf_hooks, ARRAY_SIZE(nf_hooks));
  if (ret < 0) {
    printk(KERN_ERR "Failed to register hooks: %d\n", ret);
    netlink_kernel_release(nl_sk); // 释放 netlink
    return ret;
  }

  printk(KERN_ALERT "netfilter hook demo init successfully\n");
  return NF_SUCCESS;
}

static void __exit hook_module_exit(void) {
  if (nl_sk)
    netlink_kernel_release(nl_sk);

  nf_unregister_net_hooks(&init_net, nf_hooks, ARRAY_SIZE(nf_hooks));
  printk(KERN_ALERT "netfilter hook demo exit successfully\n");
}

module_init(hook_module_init);
module_exit(hook_module_exit);
