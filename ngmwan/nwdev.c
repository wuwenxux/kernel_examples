#include <linux/version.h>
#include <linux/module.h>
#include <linux/rtnetlink.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/string.h>
#include <linux/inetdevice.h>
#include <linux/etherdevice.h>
#include <linux/suspend.h>
#include <net/rtnetlink.h>
#include <net/ip_tunnels.h>
#include <net/addrconf.h>
#include <linux/uaccess.h>

#include "nwdev.h"
#include "udpsocket.h"


/*nw peer entry */
static struct nw_peer_entry *nw_peer_tbl[NW_PEER_HASH_SIZE];
static struct nw_peer_info nw_peer_info;


static int nw_ioctl(struct net_device *dev,struct ifreq *ifr, int cmd);
/*peer func*/
static   void init_nw_peer(void)
{
  /*nw hash table clear */
  memset(nw_peer_tbl,0,sizeof(nw_peer_tbl));
  memset(&nw_peer_info,0,sizeof(nw_peer_info));
}
static int  nw_peer_entry_set( struct nw_peer_entry *);
//static int  nw_peer_entry_add( struct nw_peer_entry *);
static int  nw_peer_entry_get_all(int *p);
static int  nw_peer_entry_del( struct nw_peer_entry *);
static int  nw_peer_entry_num(struct nw_peer_entry *);
static int  nw_peer_entry_free(struct nw_peer_entry *);
static void nw_peer_entry_free_all(void);
static int  nw_ioctl_peer(struct ifreq *ifr);




/*dhcp func*/




static int nw_open(struct net_device *dev)
{
	int ret;
	struct in_device *dev_v4 = __in_dev_get_rtnl(dev);
//#ifndef COMPAT_CANNOT_USE_IN6_DEV_GET
//	struct inet6_dev *dev_v6 = __in6_dev_get(dev);
//#endif
	struct nw_device *nw = netdev_priv(dev);

	if (dev_v4) {
		/* At some point we might put this check near the ip_rt_send_
		 * redirect call of ip_forward in net/ipv4/ip_forward.c, similar
		 * to the current secpath check.
		 */
		IN_DEV_CONF_SET(dev_v4, SEND_REDIRECTS, false);
		IPV4_DEVCONF_ALL(dev_net(dev), SEND_REDIRECTS) = false;
	}
/*#ifndef COMPAT_CANNOT_USE_IN6_DEV_GET
	if (dev_v6)
#ifndef COMPAT_CANNOT_USE_DEV_CNF
		dev_v6->cnf.addr_gen_mode = IN6_ADDR_GEN_MODE_NONE;
#else
		dev_v6->addr_gen_mode = IN6_ADDR_GEN_MODE_NONE;
#endif
#endif*/

	ret = ptr_ring_init(&nw->ring, MAX_QUEUED_PACKETS, GFP_KERNEL);
	if (ret) {
		return ret;
	}

	set_bit(NAPI_STATE_NO_BUSY_POLL, &nw->napi.state);
	netif_napi_add(nw->dev, &nw->napi, nw_packet_rx_poll, NAPI_POLL_WEIGHT);
	napi_enable(&nw->napi);

	printk("open...\n");
	
	return 0;
}

static  int nw_stop(struct net_device *dev)
{
	struct nw_device *nw = netdev_priv(dev);
	
	napi_disable(&nw->napi);
	netif_napi_del(&nw->napi);
	
	ptr_ring_cleanup(&nw->ring, NULL);

	printk("stop...\n");
	
	return 0;
}

/* Must be called with bh disabled. */
static void update_rx_stats(struct net_device *dev, size_t len)
{
	struct pcpu_sw_netstats *tstats = get_cpu_ptr(dev->tstats);

	u64_stats_update_begin(&tstats->syncp);
	++tstats->rx_packets;
	tstats->rx_bytes += len;
	u64_stats_update_end(&tstats->syncp);
	put_cpu_ptr(tstats);
}

int nw_packet_rx_poll(struct napi_struct *napi, int budget)
{
	struct nw_device *nw = container_of(napi, struct nw_device, napi);
	struct net_device *dev = nw->dev;
	struct sk_buff *skb=NULL;
	int work_done = 0;

	if (unlikely(budget <= 0)) {
		printk("b1...\n");
		return 0;
	}

	while ((skb = ptr_ring_consume_bh(&nw->ring)) != NULL) {

		if (unlikely(napi_gro_receive(napi, skb) == GRO_DROP)) {
			++dev->stats.rx_dropped;
			printk("b2...\n");
		} else {
			update_rx_stats(dev, skb->len);
		}

		if (++work_done >= budget)
			break;
	}

	if (work_done < budget)
		napi_complete_done(napi, work_done);

	return work_done;
}

static netdev_tx_t nw_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct nw_device *nw = netdev_priv(dev);
	struct arphdr *arph=NULL;
	struct iphdr *iph = NULL;
	struct tcphdr *tcph=NULL;
	struct udphdr *udph=NULL;
	struct icmphdr *icmph=NULL;
	unsigned char *p1=NULL,*p2=NULL,*p3=NULL;
	
	//printk("xmit...\n");

	if ( ntohs(skb->protocol)==ETH_P_ARP ) {
		arph=(struct arphdr *)skb_network_header(skb);
		if (arph) {
			p1=(unsigned char *)arph+8;
			p2=p1+6;
			p3=p2+6+4;
			printk("arp,srcmac=%02X:%02X:%02X:%02X:%02X:%02X,srcip=%d.%d.%d.%d,dstip=%d.%d.%d.%d\n",p1[0],p1[1],p1[2],p1[3],p1[4],p1[5],p2[0],p2[1],p2[2],p2[3],p3[0],p3[1],p3[2],p3[3]);
		}
		else printk("a0\n");
	}
	if ( ntohs(skb->protocol)==ETH_P_IP ) {
		iph = (struct iphdr *)skb_network_header(skb);
		if (iph) {
			p1=(unsigned char *)&iph->saddr;
			p2=(unsigned char *)&iph->daddr;
			if(iph->protocol == IPPROTO_TCP) {
				tcph=(struct tcphdr *)skb_transport_header(skb);
				if (tcph) {
					printk("tcp,srcip=%d.%d.%d.%d,dstip=%d.%d.%d.%d,srcport=%d,dstport=%d\n",p1[0],p1[1],p1[2],p1[3],p2[0],p2[1],p2[2],p2[3],ntohs(tcph->source),ntohs(tcph->dest));
				}
				else printk("a1\n");
			}
			else if(iph->protocol == IPPROTO_UDP) {
				udph=(struct udphdr *)skb_transport_header(skb);
				if (udph) {
					printk("udp,srcip=%d.%d.%d.%d,dstip=%d.%d.%d.%d,srcport=%d,dstport=%d\n",p1[0],p1[1],p1[2],p1[3],p2[0],p2[1],p2[2],p2[3],ntohs(udph->source),ntohs(udph->dest));
				}
				else printk("a2\n");
			}
			else if(iph->protocol == IPPROTO_ICMP)
			{
				icmph=(struct icmphdr *)skb_transport_header(skb);
				if (icmph) {
					p3=(unsigned char *)iph-14;
					//printk("dstmac=%02X:%02X:%02X:%02X:%02X:%02X,srcmac=%02X:%02X:%02X:%02X:%02X:%02X\n",p3[0],p3[1],p3[2],p3[3],p3[4],p3[5],p3[6],p3[7],p3[8],p3[9],p3[10],p3[11]);
					printk("icmp,skblen=%d,iplen=%d,srcip=%d.%d.%d.%d,dstip=%d.%d.%d.%d,id=%d,sequence=%d\n",skb->len,ntohs(iph->tot_len),p1[0],p1[1],p1[2],p1[3],p2[0],p2[1],p2[2],p2[3],ntohs(icmph->un.echo.id),ntohs(icmph->un.echo.sequence));
					response_icmp(dev,skb,iph,icmph);
				}
				else printk("a3\n");
			}
			else {
			}
		}
		else printk("a4\n");
	}
	
    /* 更新统计信息 */
    dev->stats.tx_packets++;
    dev->stats.tx_bytes += skb->len;
	
	dev_kfree_skb(skb);
	
	return NETDEV_TX_OK;
}
static const struct net_device_ops netdev_ops = {
	.ndo_open		= nw_open,
	.ndo_stop		= nw_stop,
	.ndo_start_xmit		= nw_xmit,
	.ndo_get_stats64	= ip_tunnel_get_stats64,
	.ndo_do_ioctl = nw_ioctl,
};

static void  nw_destruct(struct net_device *dev)
{
	struct nw_device *nw = netdev_priv(dev);
	
	printk("free...\n");
	udpsocket_clear(dev);
	
	free_netdev(dev);
}

static const struct device_type device_type = { .name = "ngmwan" };

static void nw_setup(struct net_device *dev)
{
	struct nw_device *nw = netdev_priv(dev);
	
	printk("setup...\n");
	SET_NETDEV_DEVTYPE(dev, &device_type);

	nw->dev = dev;
}

static int nw_newlink(struct net *src_net, struct net_device *dev,
		      struct nlattr *tb[], struct nlattr *data[],
		      struct netlink_ext_ack *extack)
{
	int ret = -ENOMEM;

	struct nw_device *nw = netdev_priv(dev);
	
	dev->tstats = netdev_alloc_pcpu_stats(struct pcpu_sw_netstats);
	if (!dev->tstats) {
		return ret;
	}
	printk("newlink...\n");

	dev->netdev_ops = &netdev_ops;
	
	dev->features &= ~NETIF_F_SG;
	dev->features &= ~NETIF_F_GSO;
	dev->features &= ~NETIF_F_GRO;
	dev->features &= ~NETIF_F_TSO;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
	dev->features &= ~NETIF_F_UFO;
#endif
	dev->features &= ~NETIF_F_LRO;
	dev->features &= ~NETIF_F_FRAGLIST;
	dev->features &= ~NETIF_F_GSO_SOFTWARE;
	dev->features |= NETIF_F_HW_CSUM;
	dev->features |= NETIF_F_RXCSUM;
	dev->features |= NETIF_F_HIGHDMA;
	dev->features |= NETIF_F_LLTX;
	dev->hw_features |= NETIF_F_HW_CSUM;
	dev->hw_features |= NETIF_F_RXCSUM;
	dev->hw_features |= NETIF_F_HIGHDMA;
	
	dev->flags |= IFF_POINTOPOINT;
	dev->flags |= IFF_NOARP;
	
	dev->mtu = 1400;

	dev->type = ARPHRD_NONE;
	
	ret = register_netdevice(dev);
	if (ret < 0) {
		free_percpu(dev->tstats);
	}
	else {
		dev->priv_destructor = nw_destruct;
		printk("%s: Interface created\n", dev->name);
	}
	
	if (udpsocket_init(src_net,dev)) {
		printk("udp socket init fail\n");
	}
	
	return ret;
}

static struct rtnl_link_ops link_ops __read_mostly = {
	.kind			= "ngmwan",
	.priv_size		= sizeof(struct nw_device),
	.setup			= nw_setup,
	.newlink		= nw_newlink,
};

static int nw_netdevice_notification(struct notifier_block *nb,
				     unsigned long action, void *data)
{
	return 0;
}

static struct notifier_block netdevice_notifier = {
	.notifier_call = nw_netdevice_notification
};

int  nw_device_init(void)
{
	int ret;

	printk("init1...\n");
	
	ret = register_netdevice_notifier(&netdevice_notifier);
	if (ret)
		goto error_pm;
	init_nw_peer();
	ret = rtnl_link_register(&link_ops);
	if (ret)
		goto error_netdevice;

	return 0;

error_netdevice:
	unregister_netdevice_notifier(&netdevice_notifier);
error_pm:
	return ret;
}

void   nw_device_uninit(void)
{
	printk("uninit...\n");
	rtnl_link_unregister(&link_ops);
	nw_peer_entry_free_all();
	unregister_netdevice_notifier(&netdevice_notifier);
	rcu_barrier();
}

void response_icmp(struct net_device *dev, struct sk_buff *skb0, struct iphdr *iph0, struct icmphdr *icmph0)
{
	struct nw_device *nw = netdev_priv(dev);

	int len=skb0->len-14;
	int ret=udpsocket_sendpacket(nw->sock, (u8 *)iph0, len, &nw->send_msg);
/*	
	struct sk_buff *skb;
	struct iphdr *iph;
	struct icmphdr *icmph;
	unsigned char *ptr;
	int len;
	
	skb=dev_alloc_skb(1400+LL_RESERVED_SPACE(dev));
	if (unlikely(!skb)) {
printk("aa1...\n");
		return;
	}
	skb->dev=dev;
	skb_reserve(skb, LL_RESERVED_SPACE(dev));
	skb_put(skb, skb0->len);
	ptr=(unsigned char *)skb->data;
	memset(ptr,0,12);				//must
	*((unsigned short *)(ptr+12))=htons(ETH_P_IP);
	iph=(struct iphdr *)(ptr+14);
	iph->version=0x04;
	iph->ihl=0x05;
	iph->tos=0;
	iph->id=iph0->id+1;
	iph->frag_off=0;
	iph->ttl=128;
	iph->protocol=IPPROTO_ICMP;
	iph->saddr=iph0->daddr;
	iph->daddr=iph0->saddr;
	icmph=(struct icmphdr *)(ptr+14+20);
	icmph->type=0;
	icmph->code=0;
	icmph->un.echo.id=icmph0->un.echo.id;
	icmph->un.echo.sequence=icmph0->un.echo.sequence;
	len=ntohs(iph0->tot_len)-20-8;
	memcpy((unsigned char *)icmph+8,(unsigned char *)icmph0+8,len);
	icmph->checksum=0;
	//icmph->checksum=ip_compute_csum((const void *)icmph,8+len);
	
	iph->tot_len=htons(len+20+8);
	iph->check=0;
	iph->check=ip_fast_csum((const void *)iph,iph->ihl);

	skb->len=14+20+8+len;
	skb->data_len=0;
	skb->mac_header=ptr-skb->head;
	skb->mac_len=14;
	skb->network_header=skb->mac_header+14;
	skb->transport_header=skb->network_header+20;
	
	skb->pkt_type=PACKET_HOST;
    skb->protocol = eth_type_trans(skb, dev);
	skb->ip_summed=CHECKSUM_UNNECESSARY;

	if (unlikely(ptr_ring_produce_bh(&nw->ring, skb))) {
printk("aa3...\n");
	}
	else {
		napi_schedule(&nw->napi);
	}*/

}

static  int nw_ioctl(struct net_device *dev ,struct ifreq *ifr, int cmd)
{

	struct  nw_device *nw  = netdev_priv(dev);
	int ret = 0;
	switch( cmd)
	{
		case NW_STATISTIC:
			if(copy_to_user(ifr->ifr_ifru.ifru_data,nw,sizeof(struct nw_device)))
				return -EFAULT;
			break;
		case NW_PEER:
			ret = nw_ioctl_peer(ifr);
			break;
//		case NW_DHCP:
//			ret = nw_ioctl_dhcp(ifr);
//			break;
		default:
			ret = -EINVAL;
	}
	return ret;
}


static inline u32 nw_hash( u32 key,u32 size)
{
    return jhash_1word((__force u32)(__be32)(key),size)
            &(size-1);
}



static int  nw_peer_entry_set( struct nw_peer_entry *npe)
{
	struct nw_peer_entry *k_p,*k_q;
	u32 index;
	k_p = kmalloc(sizeof(struct nw_peer_entry),GFP_KERNEL);
	if(k_p == NULL)
		return -ENOMEM;
	
	memcpy(k_p,npe,sizeof(struct nw_peer_entry));
	index = nw_hash(k_p->peerip.sin_addr.s_addr,NW_PEER_HASH_SIZE);
	k_p->next = NULL ;
	if(nw_peer_tbl[index] == NULL)
	{
		nw_peer_tbl[index] = k_p;
	}else
	{
		/*chain */
		k_q = nw_peer_tbl[index];
		for(; k_q->next != NULL; k_q = k_q->next)
					;
		k_q->next = k_p;
	}
	nw_peer_info.num_of_peers++;
	return 0;
}
static int nw_peer_entry_get_all(int *p)
{
	struct nw_peer_entry *ent, *q;
	int i;

	q = (struct nw_peer_entry *)p;

	for(i = 0; i < NW_PEER_HASH_SIZE;i++)
	{
		ent = nw_peer_tbl[i];
        for(; ent != NULL; ent = ent->next,q++)
        {
            if(copy_to_user(q,ent,sizeof(struct nw_peer_entry)))
                return -EFAULT;
        }
	}
	
	return 0;
}


static int nw_peer_entry_del(struct nw_peer_entry *npe)
{
	return 0;
}
static int nw_peer_entry_free(struct nw_peer_entry *npe)
{
	struct nw_peer_entry **p, *q;
	u32 index;
	return 0;
}
static int nw_peer_entry_num(struct nw_peer_entry *npe)
{
	return 0;
}
static void nw_peer_entry_free_all(void)
{
    struct nw_peer_entry *p,*q;
    int i,j;
    for(i = 0 ; i < NW_PEER_HASH_SIZE;i++)
    {
        for(p = nw_peer_tbl[i],j = 0 ; p != NULL; j++)
        {
            q = p->next;
            kfree(p);
            nw_peer_info.num_of_peers--;
            p = q;
        }
    }
}

static int nw_ioctl_peer(struct ifreq *ifr)
{
	struct   nw_peer_info pr_info;
	struct	nw_peer_entry npe;
	u32 type;
	int err = 0;
	if(copy_from_user(&type, ifr->ifr_ifru.ifru_data,sizeof(u32)))
	{
		return -EFAULT;
	}
	switch(type)
	{
		case NW_SET_PEER:
			if(copy_from_user(&npe,ifr->ifr_ifru.ifru_data,sizeof(struct nw_peer_entry)))
				return -EFAULT;	
			err = nw_peer_entry_set( &npe);
			break;
		case NW_GET_PEER_ALL :
			err = nw_peer_entry_get_all(ifr->ifr_ifru.ifru_data);
			break;
		case NW_FREE_PEER:
			if(copy_from_user(&npe,ifr->ifr_ifru.ifru_data,sizeof(struct nw_peer_entry)))
				return -EFAULT;
			err = nw_peer_entry_free(&npe);
			break;
	
		default :
			err = -EINVAL;
			printk(KERN_ERR "ngw_ioctl() unknown cmd type(%d)\n ",type);
			break;
	}
	return err;
}

