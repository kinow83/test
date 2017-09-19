/*
 * procfs2.c
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

#define PROCFS_MAX_SIZE 1024
#define PROCFS_NAME "buffer1k"

struct proc_dir_entry *my_proc_file;

static char procfs_buffer[PROCFS_MAX_SIZE];

static ssize_t buffer_wr_pos = 0;
static ssize_t buffer_rd_pos = 0;

static ssize_t procfile_read2(
		struct file *file, /* see include/linux/fs.h */
		char __user *buffer, /* buffer to be filled with data */
		size_t length, /* length of the buffer */
		loff_t *offset)
{
	ssize_t left = buffer_wr_pos - buffer_rd_pos;

	if (left == 0) {
		buffer_wr_pos = 0;
		buffer_rd_pos = 0;
		return 0;
	}

	if (left > length) {
		left = length;
	}
	memcpy(buffer, procfs_buffer + buffer_rd_pos, left);

	buffer_rd_pos += left;

	printk(KERN_INFO "read [%ld/%ld] bytes (pos:%ld)\n", left, buffer_wr_pos, buffer_rd_pos);

	return left;
}

static ssize_t procfile_write2(
		struct file *file,
		const char __user *buffer,
		size_t length,
		loff_t *offset)
{
	ssize_t write_bytes = length;

	if (length == 0) {
		return 0;
	}
	if (write_bytes > PROCFS_MAX_SIZE) {
		write_bytes = PROCFS_MAX_SIZE;
	}
	if (copy_from_user(procfs_buffer, buffer, write_bytes)) {
		return -EFAULT;
	}

	printk(KERN_INFO "write [%ld/%d] bytes\n", write_bytes, PROCFS_MAX_SIZE);

	buffer_wr_pos = write_bytes;

	return write_bytes;
}

static const struct file_operations fops = {
		.owner = THIS_MODULE,
		.read = procfile_read2,
		.write = procfile_write2,
};

int init_module()
{
	struct proc_dir_entry *parent = NULL;

	my_proc_file = proc_create(PROCFS_NAME, 0644, parent, &fops);
	if (my_proc_file == NULL) {
		proc_remove(my_proc_file);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROCFS_NAME);
		return -ENOMEM;
	}
	printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
	return 0;
}

void cleanup_module()
{
	proc_remove(my_proc_file);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}

MODULE_LICENSE("GPL");
