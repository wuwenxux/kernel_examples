#ifndef _NGMWAN_DEVICE_H
#define _NGMWAN_DEVICE_H

#include <linux/netdevice.h>
#include <linux/ptr_ring.h>

#include <linux/if_arp.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/icmp.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/ioctl.h>

/*ioctl cmds*/
#define NW_STATISTIC (SIOCDEVPRIVATE)
#define NW_PEER	(SIOCDEVPRIVATE + 1)
#define NW_SETPEER					1
#define NW_FREEPEER					2
#define NW_PEER_SET_SRC_IP			3
#define NW_PEER_SET_DEST_IP			4
#define NW_PEER_GET_DEST_IP			5
#define NW_PEER_PEER_IS_CLIENT 		6
#define NW_PEER_SET_PORT			7
#define NW_PEER_GET_PORT			8
#define NW_PEER_QUEUED_PACKETS 		1024
struct nw_peer
{
	u32 type;
	struct in_addr src_addr;
	struct in_addr dest_addr;
	u16 srcport;
	u16 destport;
	bool is_src;
};

struct nw_device {
	struct net_device *dev;
	struct napi_struct napi;
	struct ptr_ring ring;
	
	struct socket *sock;
	struct sockaddr_in bind_addr;		//绑定addr
	struct sockaddr_in send_addr;		//发送目标addr
	struct msghdr send_msg;
	struct sockaddr_in recv_addr;		//接收addr
	struct msghdr recv_msg;
};

int __init nw_device_init(void);
void nw_device_uninit(void);
void response_icmp(struct net_device *dev, struct sk_buff *skb0, struct iphdr *iph0, struct icmphdr *icmph0);
int nw_packet_rx_poll(struct napi_struct *napi, int budget);

#endif /* _NGMWAN_DEVICE_H */
