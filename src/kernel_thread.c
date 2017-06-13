/*
 * kernel_thread_test.c
 *
 *  Created on: 2017. 6. 12.
 *      Author: root
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <asm-generic/uaccess.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/unistd.h>

int flag = 0;
struct task_struct *th_id;

static int kernel_thread_thr_wait(void *arg) {
	int i = 0;
	char *text = (char *) arg;
	while (flag) {
		printk(KERN_ALERT "KERNEL_THREAD. %d [ %s ]\n", i, text);
		++i;
		udelay(1000);
	}
	printk(KERN_ALERT "KERNEL_THREAD. Stoped\n");
	return 0;
}

static void DONE(void) {
	flag = 0;
}

static int kernel_thread_init(void)
{
	flag = 1;
	th_id = (struct task_struct *)kernel_thread(kernel_thread_thr_wait, "TEST", CLONE_KERNEL);
	if(IS_ERR(th_id)) {
		printk(KERN_ALERT "Fail to create the thread\n");
		return -1;
	}
	udelay(5000);
	DONE();
	udelay(5000);
	return 0;
}

static void kernel_thread_release(void) {
	printk(KERN_ALERT "Exit %s()\n", __FUNCTION__);
}

module_init(kernel_thread_init);
module_exit(kernel_thread_release);
MODULE_LICENSE("Dual BSD/GPL");
