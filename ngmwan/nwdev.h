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
#define NW_SET_PEER						1
#define NW_GET_PEER						2
#define NW_FREE_PEER					3
#define NW_ADD_PEER						4
#define NW_DEL_PEER						5
#define NW_GET_PEER_NUMS				6
#define NW_SET_DEFPEER					7
#define NW_DHCP (SIOCDEVPRIVATE + 2)

/*COMMON  */
#define MAX_QUEUED_PACKETS 1024
typedef enum mode
{
    client,
    server
}mode_t ;
const char * mode_tbl = {"client","server", NULL};
inline mode_t find_mode(char *sval)
{
	mode_t result = client;
	int i = 0;
	for (i = 0; mode_tbl[i] != NULL ; ++i,++result)
		if(0== strcmp(sval,etable[i]))
			return result;
	return -1;
}
struct nw_peer_entry
{
	u32 type;
	struct nw_peer_entry *next;
	struct in_addr bindip;
	int bindport;
	u32 peer_id;
	mode_t cli_server; // cli or ser
};
struct nw_peer_info
{
	u32 type;
	u32 num_of_peers;
	
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
