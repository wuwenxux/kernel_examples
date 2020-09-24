/*
 * hello.c: Creates a read−only char device that says how many times
 * you've read from the dev file
 */
#include<linux/init.h>  //Macros used to mark up functions e.g. __init __exit
#include<linux/kernel.h> //Contains types, macros, functions for the kernel
#include<linux/device.h> //Header to support the kernel Driver Model
#include<linux/module.h> //Core header 
#include<linux/fs.h>     //Header for the Linux file system support
#include<asm/uaccess.h>  /*for put_user*/
/*
 * Prototypes - this would normally go in a .h file
 *
 */
#define DEVICE_NAME "hellochar"
#define SUCCESS 0
#define BUF_LEN 80 		/*Max length of the message from the device */


static int device_open(struct inode *, struct file*);
static int device_release(struct inode *,struct file*);
static ssize_t device_read(struct file *, char *, size_t ,loff_t *);
static ssize_t device_write(struct file *,const char *, size_t ,loff_t *);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wafer");
MODULE_DESCRIPTION("A simple char dev.");
MODULE_VERSION("0.1");

/*
 *Global variables are declared as static , so are global within the file.
 */
static int Major;
static int Device_open = 0;	/*Is device open?
				 *Used to prevent multiple access to device */
static char msg[BUF_LEN]={0};	/*The msg the device will give when asked */
static char *msg_Ptr;
//static int numberOpens = 0;

//static struct class* helloClass = NULL;
//static struct device* helloDevice = NULL;
static struct file_operations fops= {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

/*
 *This function is called when the module is loaded
 */
int __init hello_init(void)
{	
	Major = register_chrdev(0,DEVICE_NAME,&fops);

	if(Major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}
	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver ,create a dev file with\n");
	printk(KERN_INFO "'mknode /dev/%s c %d 0'.\n",DEVICE_NAME,Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
 	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");
	printk(KERN_INFO "Hello: device class registered correctly\n");
        return SUCCESS;
}
/*
 *This function is called when the module is unloaded
 */
void __exit hello_exit(void) 
{
	unregister_chrdev(Major,DEVICE_NAME);     //unregister the major number
	printk(KERN_INFO "hello: Goodbye \n");

}

/*
 * Called when a process tries to open the device file, like
 * "cat /dev/hellochar"
 */
static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 0;
	if (Device_open)
		return -EBUSY;
	Device_open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);
 	return 	SUCCESS;
}
/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
	Device_open--;
       	/* We're now ready for our next caller */
	 /*
	 * Decrement the usage count, or else once you opened the file, you'll
	 * never get get rid of the module.
	 */
  	module_put(THIS_MODULE);
  	return 0;
}
/*
 * Called when a process ,which already opened the dev file ,attempts to
 * read from it.
 */

static ssize_t device_read(struct file *filp,
			   char *buffer,
			   size_t length,
			   loff_t *offset)
{
	/*
	 *Number of bytes actually written to the buffer
	 */
	int bytes_read = 0;
	
	/*
	 *If we're at the end of the message,
	 *return 0 signifying end of file
	 */
	if(*msg_Ptr == 0)
		return 0;

	/*
	 *If we're at the end of the message,
	 *return 0 signifying end of file
	 */
	while(length&& *msg_Ptr){
		put_user(*(msg_Ptr++),buffer++);

		length--;
		bytes_read++;
	}
	return bytes_read;
}

/*
 *Called when a process writes to dev file:echo "hi">/dev/hello
 */
static ssize_t
device_write(struct file *filp,const char *buff, size_t len, loff_t *off)
{
	printk(KERN_ALERT "Sorry,this operation isn't supported.\n");
	return -EINVAL;
}
module_init(hello_init);
module_exit(hello_exit);
