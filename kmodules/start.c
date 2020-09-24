/*
 *
 *start.c - Illustration of multi filed modules
 */

#include <linux/kernel.h>
#include <linux/module.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wafer  wuwenxuan@baicells.com");

int init_module(void)
{
	printk(KERN_INFO "Hello, world − this is the kernel speaking\n");
 	return 0;
}

