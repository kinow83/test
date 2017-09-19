/*
 * kernel_ioctl.c
 *
 *  Created on: 2016. 10. 14.
 *      Author: root
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/stat.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>


#define DEV_NUM 200
#define DEV_NAME "virtual_device"
#define MY_IOCTL_NUMBER 100
#define SUCCESS 0


static int data = 0;

static int virtual_device_open(struct inode *node, struct file *file)
{
	pr_info("virtual device open function call\n");
	return SUCCESS;
}

static int virtual_device_release(struct inode *node, struct file *file)
{
	pr_info("virtual device relase function call\n");
	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
#define MY_IOC_READ      _IOR( MY_IOCTL_NUMBER, 0, int )
#define MY_IOC_WRITE     _IOW( MY_IOCTL_NUMBER, 1, int )
#define MY_IOC_STATUS    _IO ( MY_IOCTL_NUMBER, 2)
#define MY_IOC_READWRITE _IOWR(MY_IOCTL_NUMBER, 3, int)
#define MY_IOC_NUMBER    4

////////////////////////////////////////////////////////////////////////////////
static DEFINE_MUTEX(extio_mutex);

static long virtual_device_ioctl(
		struct file *file,
		unsigned int cmd,  // ioctl type
		unsigned long arg) // user data memory
{
	int err = 0;
	int size;

	// check cmd of ioctl that is my device
	if ( _IOC_TYPE(cmd) != MY_IOCTL_NUMBER ) {
		pr_alert("ioctl type invalid %04x   %d\n", cmd, _IOC_TYPE(cmd));
		return -EINVAL;
	}
	if ( _IOC_NR(cmd)   >= MY_IOC_NUMBER   ) {
		pr_alert("ioctl cmd invalid %d\n", _IOC_NR(cmd));
		return -EINVAL;
	}

	if (_IOC_DIR(cmd) & _IOC_READ) {
		// Can read memory with ioctl
		err = access_ok( VERIFY_READ, (void*)arg, sizeof(unsigned long) );
		if (err < 0) {
			pr_alert("access failed VERIFY_READ = %p\n", (void*)arg);
			return -EINVAL;
		}
	}
	else if (_IOC_DIR(cmd) & _IOC_WRITE) {
		// Can write memory with ioctl
		err = access_ok( VERIFY_WRITE, (void*)arg, sizeof(unsigned long) );
		if (err < 0) {
			pr_alert("access failed VERIFY_WRITE = %p\n", (void*)arg);
			return -EINVAL;
		}
	}
	size = _IOC_SIZE( cmd );

	mutex_lock(&extio_mutex);
	switch (cmd)
	{
	case MY_IOC_READ:
		pr_info("[IOCTL READ] called\n");
		copy_to_user( (void*)arg, (const void*)&data, (unsigned long)size );
		pr_info("[IOCTL READ] data in kernel space: %d\n", data);
		break;

	case MY_IOC_WRITE:
		pr_info("[IOCTL WRITE] called\n");
		copy_from_user( (void*)&data, (const void*)arg, (unsigned long)size );
		pr_info("[IOCTL WRITE] data in kernel space: %d\n", data);
		break;

	case MY_IOC_STATUS:
		pr_info("[IOCTL STATUS] called\n");
		break;

	case MY_IOC_READWRITE:
		pr_info("[IOCTL READ WRITE] called\n");

		copy_from_user( (void*)&data, (const void*)arg, (unsigned long)size );
		pr_info("[IOCTL READ WRITE] data in kernel space: %d\n", data);

		data += 900;

		copy_to_user( (void*)arg, (const void*)&data, (unsigned long)size );
		pr_info("[IOCTL READ WRITE] data in kernel space: %d\n", data);

		break;

	default:
		pr_err("IOCTL] unknown ioctl\n");
		break;
	}
	mutex_unlock(&extio_mutex);

	return 0;
}

static struct file_operations vd_fops = {
		.owner = THIS_MODULE,
		.open = virtual_device_open,
		.release = virtual_device_release,
		.unlocked_ioctl = virtual_device_ioctl
};

int __init virtual_device_init( void )
{
	int ret;
	ret = register_chrdev( DEV_NUM, DEV_NAME, &vd_fops);
	if (ret < 0) {
		pr_alert("driver init failed\n");
		return ret;
	} else {
		pr_alert("driver init success\n");
		return SUCCESS;
	}
}

void __exit virtual_device_exit( void )
{
	unregister_chrdev( DEV_NUM, DEV_NAME );
	pr_alert("driver cleanup success\n");
}

module_init(virtual_device_init);
module_exit(virtual_device_exit);

MODULE_LICENSE("GPL");



