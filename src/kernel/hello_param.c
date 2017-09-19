/*
 * hello_param.c
 *
 *  Created on: 2016. 10. 4.
 *      Author: root
 */




#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>


static short int myshort = 1;
static       int myint = 420;
static long  int mylong = 9999;
static char* mystring = "kaka";
static int myintArray[2] = {-1, -1};
static int arr_argc = 0;


module_param(myshort, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(myshort, "A short integer");



static int __init hello_param_init(void)
{
	int i;
	printk(KERN_INFO "hello, param\n");
	printk(KERN_INFO "myshort is a short integer: %hd\n", myshort);

	printk(KERN_INFO "got %d arguments for myintArray.\n", arr_argc);
	return 0;
}

static void __exit hello_param_exit(void)
{
	printk(KERN_INFO "Gooby, hell param\n");
}


module_init(hello_param_init);
module_exit(hello_param_exit);
