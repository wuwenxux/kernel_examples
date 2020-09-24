#include <linux/inet.h>
#include "udpsocket.h"
#include "device.h"


int udpsocket_init(struct net *src_net, struct net_device *dev)
{
	struct nw_device *nw = netdev_priv(dev);
	
	int ret=0;
	struct socket *sock=NULL;
	struct sock *sk;
	
	nw->sock=NULL;
	//ret = sock_create_kern(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock);
	ret = __sock_create(src_net, AF_INET, SOCK_DGRAM, 0, &sock, 1);
	if (ret < 0) {
		printk("create udp socket fail\n");
		ret=-1;
		return ret;
	}

	memset(&nw->bind_addr, 0, sizeof(struct sockaddr_in));
	nw->bind_addr.sin_family = AF_INET;
	nw->bind_addr.sin_port = htons(18222);
	nw->bind_addr.sin_addr.s_addr = in_aton("172.19.1.12");		//htonl(INADDR_ANY)
	if (kernel_bind(sock, (struct sockaddr *)&nw->bind_addr, sizeof(struct sockaddr_in)) < 0) {
		kernel_sock_shutdown(sock, SHUT_RDWR);
		sock_release(sock);
		printk("udp socket bind fail\n");
		ret=-1;
		return ret;
	}
	
	memset(&nw->send_addr, 0, sizeof(struct sockaddr_in));
	nw->send_addr.sin_family = AF_INET;
	nw->send_addr.sin_port = htons(822);
	nw->send_addr.sin_addr.s_addr = in_aton("172.19.1.123");
	memset(&nw->send_msg, 0, sizeof(struct msghdr));
	nw->send_msg.msg_name=(struct sockaddr *)&nw->send_addr;
	nw->send_msg.msg_namelen=sizeof(struct sockaddr_in);
	if ( kernel_connect(sock, (struct sockaddr *)&nw->send_addr, sizeof(struct sockaddr_in), 0) <0 ) {
		kernel_sock_shutdown(sock, SHUT_RDWR);
		sock_release(sock);
		printk("udp socket connect fail\n");
		ret=-1;
		return ret;
	}

	sk = sock->sk;
	sk->sk_allocation = GFP_ATOMIC;
	sk->sk_sndbuf = INT_MAX;
	//sk_set_memalloc(sk);
	rcu_assign_sk_user_data(sk, (void *)nw);		//把nw指针设置到sk，读取回调函数中可以取出nw
	sk->sk_data_ready = udpsocket_data_ready;		//设置数据包读取的回调函数
	synchronize_rcu();
	synchronize_net();
	
	nw->sock=sock;

	memset(&nw->recv_msg, 0, sizeof(struct msghdr));
	nw->recv_msg.msg_name=(struct sockaddr *)&nw->recv_addr;
	nw->recv_msg.msg_namelen=sizeof(struct sockaddr_in);

	printk("create udp socket ok\n");
	
	ret=0;
	return ret;
}

void udpsocket_clearsocket(struct socket *sock)
{
	//sk_clear_memalloc(sock->sk);
	rcu_assign_sk_user_data(sock->sk, NULL);
	kernel_sock_shutdown(sock, SHUT_RDWR);
	sock_release(sock);
}

void udpsocket_clear(struct net_device *dev)
{
	struct nw_device *nw = netdev_priv(dev);

	if (nw->sock) {
		udpsocket_clearsocket(nw->sock);
		nw->sock=NULL;
	}
}

int udpsocket_sendpacket(struct socket *sock, u8 *data, int datasize, struct msghdr *sendmsg)
{
	int len,i;
	struct kvec iov;
	
	if (unlikely(!sock)) return -1;
	
	sendmsg->msg_flags = MSG_DONTWAIT|MSG_NOSIGNAL;
	iov.iov_base = (void *)data;
	iov.iov_len = datasize;
	len = kernel_sendmsg(sock, sendmsg, &iov, 1, datasize);
	if (len>0) {
		printk("send ok,len=%d,data=",len);
		for (i=0;i<len;i++) {
			if (i==0) printk("%02X",data[i]);
			else printk(",%02X",data[i]);
		}
		printk("\n");
	}
	else {
		printk("send fail,ret=%d...\n",len);
	}
	return len;
}

void udpsocket_data_ready(struct sock *sk)
{
	struct sk_buff *skb;
	while ((skb = skb_dequeue(&sk->sk_receive_queue)) != NULL) {
		skb_orphan(skb);
		sk_mem_reclaim(sk);
		udpsocket_recvpacket(sk, skb);
	}
}

void udpsocket_recvpacket(struct sock *sk, struct sk_buff *skb)
{
	int ret=-1;
	struct iphdr *iph = NULL;
	//struct tcphdr *tcph=NULL;
	struct udphdr *udph=NULL;
	//struct icmphdr *icmph=NULL;
	unsigned char *p1=NULL,*p2=NULL,*p3=NULL;
	int len,iplen,i;
	struct nw_device *nw = (struct nw_device *)sk->sk_user_data;
	
	if (unlikely(!nw)) {
		kfree_skb(skb);
		return;
	}
	if (unlikely(!nw->sock)) {
		kfree_skb(skb);
		return;
	}
	
	if ( ntohs(skb->protocol)==ETH_P_IP ) {
		iph = (struct iphdr *)skb_network_header(skb);
		if (iph) {
			p1=(unsigned char *)&iph->saddr;
			p2=(unsigned char *)&iph->daddr;
			if(iph->protocol == IPPROTO_UDP) {
				udph=(struct udphdr *)skb_transport_header(skb);
				if (udph) {
					iplen=iph->ihl*4;
					len=ntohs(iph->tot_len);
					printk("recv udp,skblen=%d,datalen=%d,iplen=%d,srcip=%d.%d.%d.%d,dstip=%d.%d.%d.%d,srcport=%d,dstport=%d,data=",skb->len,skb->data_len,len,p1[0],p1[1],p1[2],p1[3],p2[0],p2[1],p2[2],p2[3],ntohs(udph->source),ntohs(udph->dest));
					len -= (iplen+8);
					p3=(unsigned char *)udph+8;
					for (i=0;i<len;i++) {
						if (i==0) printk("%02X",p3[i]);
						else printk(",%02X",p3[i]);
					}
					printk("\n");
				}
			}
		}
	}

	if (ret) {
		//dev->stats.rx_errors++;
		//dev->stats.rx_frame_errors++;
		dev_kfree_skb(skb);
	}
}
