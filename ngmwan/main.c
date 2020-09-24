#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/genetlink.h>
#include <net/rtnetlink.h>

#include "device.h"
#include "netlink.h"

static int __init init_ngmwan(void)
{
	int ret;
	printk("init...\n");
	ret = nw_device_init();
	if (ret < 0)
		goto err_device;
	
	ret = nw_genetlink_init();
	if (ret < 0)
		goto err_netlink;
	
	return 0;
	
err_netlink:
	nw_device_uninit();
err_device:
	return ret;
}
 
static void __exit clear_ngmwan(void)
{
	nw_genetlink_uninit();
	nw_device_uninit();
	
	printk("exit...\n");
}
 
module_init(init_ngmwan);
module_exit(clear_ngmwan);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Baicelss");
MODULE_DESCRIPTION("ng multi wan");
MODULE_VERSION("1.0.2020.0730");
MODULE_ALIAS_RTNL_LINK(KBUILD_MODNAME);
MODULE_ALIAS_GENL_FAMILY(NGMWAN_GENL_NAME);
