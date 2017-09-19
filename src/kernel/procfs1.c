/*
 * procfs1.c
 *
 *  Created on: 2016. 10. 6.
 *      Author: root
 */



#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/stat.h>

/* Specifically, a module */
/* We're doing kernel work */
/* Necessary because we use the proc fs */
#define procfs_name "helloworld"


#if 0
struct proc_dir_entry {
    unsigned int low_ino;
    umode_t mode;
    nlink_t nlink;
    kuid_t uid;
    kgid_t gid;
    loff_t size;
    const struct inode_operations *proc_iops;
    const struct file_operations *proc_fops;
    struct proc_dir_entry *parent;
    struct rb_root subdir;
    struct rb_node subdir_node;
    void *data;
    atomic_t count;     /* use count */
    atomic_t in_use;    /* number of callers into module in progress; */
            /* negative -> it's going away RSN */
    struct completion *pde_unload_completion;
    struct list_head pde_openers;   /* who did ->open, but not ->release */
    spinlock_t pde_unload_lock; /* proc_fops checks and pde_users bumps */
    u8 namelen;
    char name[];
};
#endif
struct proc_dir_entry *my_proc_file;

static char message[] = "Hello World\n";
static char *message_ptr = message;

static ssize_t procfile_read(
		struct file *file, /* see include/linux/fs.h */
		char __user *buffer, /* buffer to be filled with data */
		size_t length, /* length of the buffer */
		loff_t *offset)
{
	int bytes_read = 0;
	if (*message_ptr == 0) {
		return 0;
	}

	printk(KERN_INFO "profile_read (/proc/%s) called\n", procfs_name);
	printk(KERN_INFO "file:%p, buffer:%p, length:%ld, offset:%p\n",
			file, buffer, length, offset);

	while (length && *message_ptr) {
		put_user(*(message_ptr++), buffer++);
		length--;
		bytes_read++;
	}
	printk(KERN_INFO "read [%d] bytes, [%ld] left\n", bytes_read, length);
	return bytes_read;
}

static const struct file_operations fops = {
		.owner = THIS_MODULE,
		.read = procfile_read,
};

int init_module()
{
	struct proc_dir_entry *parent = NULL;

	my_proc_file = proc_create(procfs_name, 0644, parent, &fops);
	if (my_proc_file == NULL) {
		proc_remove(my_proc_file);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", procfs_name);
		return -ENOMEM;
	}
	printk(KERN_INFO "/proc/%s created\n", procfs_name);
	return 0;
}

void cleanup_module()
{
	proc_remove(my_proc_file);
	printk(KERN_INFO "/proc/%s removed\n", procfs_name);
}

MODULE_LICENSE("GPL");
