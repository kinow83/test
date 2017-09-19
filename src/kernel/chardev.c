/*
 * char.c
 *
 *  Created on: 2016. 10. 4.
 *      Author: root
 */

#include <linux/kernel.h> /* We're doing kernel work */
#include <linux/module.h> /* Specifically, a module */
#include <linux/fs.h>
#include <asm/uaccess.h> /* for get_user and put_user */

#include "chardev.h"

#define SUCCESS 0
#define BUF_LEN 80

static int device_opened = 0;
static char message[BUF_LEN];
static char *message_ptr;



static int device_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "device_open(\n"
			"file:%p)\n", file);

	if (device_opened) {
		return -EBUSY;
	}
	device_opened++;

	message_ptr = message;
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "device_release(\n"
			"inode:%p,\n"
			"file:%p)\n",
			inode, file);

	device_opened--;

	module_put(THIS_MODULE);
	return SUCCESS;
}

static ssize_t device_read(
		struct file *file, /* see include/linux/fs.h */
		char __user *buffer, /* buffer to be filled with data */
		size_t length, /* length of the buffer */
		loff_t *offset)
{
	int bytes_read = 0;
	printk(KERN_INFO "device_read(\n"
			"file:%p,\n"
			"user-buffer:%p,\n"
			"user-buffer-length:%ld)\n",
			file, buffer, length);

	if (*message_ptr == 0) {
		return 0;
	}

	while (length && *message_ptr) {
		put_user(*(message_ptr++), buffer++);
		length--;
		bytes_read++;
	}
	printk(KERN_INFO "read [%d] bytes, [%ld] left\n", bytes_read, length);
	return bytes_read;
}

static ssize_t device_write(
		struct file *file,
		const char __user *buffer,
		size_t length,
		loff_t *offset)
{
	int i;
	printk(KERN_INFO "device_write(\n"
			"file:%p,\n"
			"user-buffer:%p,\n"
			"user-buffer-length:%ld)\n",
			file, buffer, length);

	for (i=0; i<length && i<BUF_LEN; i++) {
		get_user(message[i], buffer+i);
	}
	return i;
}

long device_ioctl(
		struct file *file, /* see include/linux/fs.h */
		unsigned int ioctl_num, /* number and param for ioctl */
		unsigned long ioctl_param)
{
	int i;
	char *temp;
	char ch;

	printk(KERN_INFO "device_ioctl(\n"
			"file:%p,\n"
			"ioctl-num:%ld,\n"
			"ioctl-param:0x%lx)\n",
			file, ioctl_num, ioctl_param);

	switch (ioctl_num) {
	case IOCTL_SET_MSG:
		temp = (char *)ioctl_param;

		get_user(ch, temp);

		for (i=0; ch && i<BUF_LEN; i++, temp++) {
			get_user(ch, temp);
		}

		device_write(file, (char*)ioctl_param, i, 0);

		break;

	case IOCTL_GET_MSG:
		i = device_read(file, (char*)ioctl_param, BUF_LEN, 0);

		// NULL(zero) is terminated
		put_user('\0', ((char*)ioctl_param)+i);

		break;

	case IOCTL_GET_NTH_BYTE:
		return message[ioctl_param];
		break;
	}
	return SUCCESS;
}

struct file_operations fops = {
		.read = device_read,
		.write = device_write,
		.unlocked_ioctl = device_ioctl,
		.open = device_open,
		.release = device_release,
};

int init_module()
{
	int ret;

	ret = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);

	if (ret < 0) {
		printk(KERN_ALERT "%s failed with %d\n",
				"Sorry, registering the character device", ret);
		return ret;
	}
	printk(KERN_INFO "%s The major device number is %d.\n",
			"Registeration is a success", MAJOR_NUM);
	printk(KERN_INFO "If you want to talk to the device driver, \n");
	printk(KERN_INFO "you'll have to create a device file. \n");
	printk(KERN_INFO "We suggest you use: \n");
	printk(KERN_INFO "mknod %s c %d 0\n", DEVICE_NAME, MAJOR_NUM);
	printk(KERN_INFO "The device file name is important, because\n");
	printk(KERN_INFO "the ioctl program assumes that's the\n");
	printk(KERN_INFO "file you'll use.\n");
	return 0;
}

void cleanup_module()
{
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
	printk(KERN_ALERT "Error: unregister_chadev: %d, %s\n", MAJOR_NUM, DEVICE_NAME);
}
