/*
 *
 *start.c - Illustration of multi filed modules
 */

#include <linux/kernel.h>
#include <linux/module.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wafer  wuwenxuan@baicells.com");

void cleanup_module()
{
	printk(KERN_INFO "Short is the  life of a kernel module\n");
}

