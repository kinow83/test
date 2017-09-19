
#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/init.h> /* Needed for the macros */

#define DRIVER_AUTHOR "kinow@naver.com"
#define DRIVER_DESC "A sample driver"

static int hello2_data __initdata = 2;

static int __init hello_2_init(void) {
	printk(KERN_INFO "Hello, world %d\n", hello2_data);
	return 0;
}

static void __exit hello_2_exit(void) {
	printk(KERN_INFO "Goodbyte, world 2\n");
}

module_init(hello_2_init);
module_exit(hello_2_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR); /* Who wrote this module? */
MODULE_DESCRIPTION(DRIVER_DESC); /* What does this module do */
MODULE_SUPPORTED_DEVICE("testdevice");
