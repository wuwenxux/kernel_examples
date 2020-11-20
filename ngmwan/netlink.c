#include <linux/if.h>
#include <net/sock.h>

#include "compat.h"
#include "nwdev.h"
#include "netlink.h"

static struct genl_family genl_family;

static const struct nla_policy device_policy[NWDEVICE_A_MAX + 1] = {
	[NWDEVICE_A_IFINDEX]		= { .type = NLA_U32 },
	[NWDEVICE_A_IFNAME]		= { .type = NLA_NUL_STRING, .len = IFNAMSIZ - 1 },
};

static struct nw_device *lookup_interface(struct nlattr **attrs,
					  struct sk_buff *skb)
{
	struct net_device *dev = NULL;

	if (!attrs[NWDEVICE_A_IFINDEX] == !attrs[NWDEVICE_A_IFNAME])
		return ERR_PTR(-EBADR);
	if (attrs[NWDEVICE_A_IFINDEX])
		dev = dev_get_by_index(sock_net(skb->sk),
				       nla_get_u32(attrs[NWDEVICE_A_IFINDEX]));
	else if (attrs[NWDEVICE_A_IFNAME])
		dev = dev_get_by_name(sock_net(skb->sk),
				      nla_data(attrs[NWDEVICE_A_IFNAME]));
	if (!dev)
		return ERR_PTR(-ENODEV);
	if (!dev->rtnl_link_ops || !dev->rtnl_link_ops->kind ||
	    strcmp(dev->rtnl_link_ops->kind, KBUILD_MODNAME)) {
		dev_put(dev);
		return ERR_PTR(-EOPNOTSUPP);
	}
	return netdev_priv(dev);
}

struct dump_ctx {
	struct nw_device *nw;
	u64 allowedips_seq;
	struct allowedips_node *next_allowedip;
};

#define DUMP_CTX(cb) ((struct dump_ctx *)(cb)->args)

static int nw_get_device_start(struct netlink_callback *cb)
{
	struct nw_device *nw;

	printk("devicestart...\n");
	
	nw = lookup_interface(genl_dumpit_info(cb)->attrs, cb->skb);
	if (IS_ERR(nw))
		return PTR_ERR(nw);
	DUMP_CTX(cb)->nw = nw;
	return 0;
}

static int nw_get_device_dump(struct sk_buff *skb, struct netlink_callback *cb)
{
	struct dump_ctx *ctx = DUMP_CTX(cb);
	struct nw_device *nw = ctx->nw;
	int ret = -EMSGSIZE;
	void *hdr;

	printk("detdevicedump...\n");
	
	hdr = genlmsg_put(skb, NETLINK_CB(cb->skb).portid, cb->nlh->nlmsg_seq,
			  &genl_family, NLM_F_MULTI, NW_CMD_GET_DEVICE);
	if (!hdr)
		goto out;
	genl_dump_check_consistent(cb, hdr);
	
	ret = 0;

out:
	if (ret) {
		genlmsg_cancel(skb, hdr);
		return ret;
	}
	genlmsg_end(skb, hdr);
	return skb->len;
}

static int nw_get_device_done(struct netlink_callback *cb)
{
	struct dump_ctx *ctx = DUMP_CTX(cb);

	if (ctx->nw)
		dev_put(ctx->nw->dev);
	
	printk("getdevice...\n");
	
	return 0;
}

static int nw_set_device(struct sk_buff *skb, struct genl_info *info)
{
	struct nw_device *nw = lookup_interface(info->attrs, skb);
	int ret=0;

	printk("setdevice...\n");
	
	dev_put(nw->dev);
	
	return ret;
}

#ifndef COMPAT_CANNOT_USE_CONST_GENL_OPS
static const
#else
static
#endif
struct genl_ops genl_ops[] = {
	{
		.cmd = NW_CMD_GET_DEVICE,
#ifndef COMPAT_CANNOT_USE_NETLINK_START
		.start = nw_get_device_start,
#endif
		.dumpit = nw_get_device_dump,
		.done = nw_get_device_done,
#ifdef COMPAT_CANNOT_INDIVIDUAL_NETLINK_OPS_POLICY
		.policy = device_policy,
#endif
		.flags = GENL_UNS_ADMIN_PERM
	}, {
		.cmd = NW_CMD_SET_DEVICE,
		.doit = nw_set_device,
#ifdef COMPAT_CANNOT_INDIVIDUAL_NETLINK_OPS_POLICY
		.policy = device_policy,
#endif
		.flags = GENL_UNS_ADMIN_PERM
	}
};

static struct genl_family genl_family
#ifndef COMPAT_CANNOT_USE_GENL_NOPS
__ro_after_init = {
	.ops = genl_ops,
	.n_ops = ARRAY_SIZE(genl_ops),
#else
= {
#endif
	.name = NGMWAN_GENL_NAME,
	.version = NGMWAN_GENL_VERSION,
	.maxattr = NWDEVICE_A_MAX,
	.module = THIS_MODULE,
#ifndef COMPAT_CANNOT_INDIVIDUAL_NETLINK_OPS_POLICY
	.policy = device_policy,
#endif
	.netnsok = true
};

int __init nw_genetlink_init(void)
{
	printk("link_init...\n");
	return genl_register_family(&genl_family);
}

void __exit nw_genetlink_uninit(void)
{
	printk("link_uninit...\n");
	genl_unregister_family(&genl_family);
}
