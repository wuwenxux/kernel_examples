#include "ngwdev.h"

#include <linux/netdevice.h>
#include <linux/init.h>
static int ngw_open(struct net_device *dev)
{
    struct ngw_device *ngw = netdev_priv(dev);
    int ret;
    
    return ret;
}
static int ngw_stop(struct net_device *dev)
{
    struct ngw_device *ngw = netdev_priv(dev);
    return 0;
}
static netdev_tx_t ngw_xmit(struct sk_buff *skb,struct net_device *dev)
{
    return 0;
}
static const struct net_device_ops netdev_ops=
{
    .ndo_open = ngw_open,
    .ndo_stop  = ngw_stop,
    .ndo_start_xmit  = ngw_xmit
};
static void destruct (struct net_device *dev)
{
    free_netdev(dev);
}
static const struct device_type device_type = { .name = KBUILD_MODNAME };
static void setup( struct net_device *dev){
        struct  ngw_device *ngw = netdev_priv(dev);
        enum{WG_NETDEV_FEATURES = NETIF_F_HW_CSUM|NETIF_F_RXCSUM|NETIF_F_SG|NETIF_F_GSO};
}
int __init ngw_device_init(void)
{
    return 0;
}
void ngw_device_uninit(void);

