#ifndef _NGMWAN_UDPSOCKET_H
#define _NGMWAN_UDPSOCKET_H

#include <linux/if_arp.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/icmp.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/in.h>

int udpsocket_init(struct net *src_net, struct net_device *dev);
void udpsocket_clearsocket(struct socket *sock);
void udpsocket_clear(struct net_device *dev);
int udpsocket_sendpacket(struct socket *sock, u8 *data, int datasize, struct msghdr *sendmsg);
void udpsocket_data_ready(struct sock *sk);
void udpsocket_recvpacket(struct sock *sk, struct sk_buff *skb);

#endif /* _NGMWAN_UDPSOCKET_H */
