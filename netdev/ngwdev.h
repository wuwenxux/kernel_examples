#ifndef _NGW_DEVICE_H
#define _NGW_DEVICE_H
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/rtnetlink.h>
#include <linux/if_arp.h>    /* ARPHRD_ETHER */
#include <linux/errno.h>     /* error codes */
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <linux/udp.h>
MODULE_AUTHOR("wafer");
MODULE_LICENSE("GPL");

struct ngw_device;

struct ngw_device
{
	struct net_device *dev;
	struct mutex device_update_lock, socket_update_lock;
	struct list_head device_list;
	u32 fwmark;
	u16 incoming_port;
};

int ngw_device_init(void);
void ngw_device_uninit(void);

#endif
