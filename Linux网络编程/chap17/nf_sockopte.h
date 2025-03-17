#ifndef __NF_SOCKOPTE_H__
#define __NF_SOCKOPTE_H__

/*
cmd 命令定义:
SOE_BANDIP: IP 地址发送禁止命令
SOE_BANDPORT: 端口禁止命令
SOE_BANDPING: ping 禁止
*/

#define SOE_BANDIP 0x6001
#define SOE_BANDPORT 0x6002
#define SOE_BANDPING 0x6003

// 禁止端口结构
typedef struct nf_bandport {
  // band protol , TCP or UDP
  unsigned short protocol;

  // band port
  unsigned short port;
} nf_bandport;

// 与用户交互的数据结构
typedef struct band_status {
  // IP 发送禁止, IP 地址, 0 表示未设置
  unsigned int band_ip;

  // 端口禁止, 协议和端口均为 0 时表示未设置
  nf_bandport band_port;

  // 是否允许 ping 回显响应, 0 响应, 1 禁止
  unsigned char band_ping;
} band_status;

#endif