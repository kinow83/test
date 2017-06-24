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

#define THREAD_MAX  3

static struct task_struct *tid[THREAD_MAX];

static volatile long count = 0;

static int thread_fn(void *data) {
	int i = 0;
	while (!kthread_should_stop()) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(HZ);
		for (i=0; i<1000; i++) {
			count++;
			count--;
		}

		printk(KERN_INFO "kinow: %d kthread running [%ld]: %s\n", current->pid, count, __FUNCTION__);

	}
	return 0;
}

static void kthread_release(void) {
	int i;
	for (i=0; i<THREAD_MAX; i++) {
        if (tid[i] && !task_is_stopped(tid[i])) {
		    kthread_stop(tid[i]);
        }
	}
}

static int kthread_init(void) {
	int i;
	int err = -EINVAL;

	memset(tid, 0, sizeof(struct task_struct *) * THREAD_MAX);

	for (i=0; i<THREAD_MAX; i++) {
		tid[i] = kthread_run(thread_fn, NULL, "counter-%d", i+1);
		if (IS_ERR(tid[i])) {
			goto out;
		}
	}
	printk(KERN_INFO "init kthread success\n");
	return 0;
out:
	printk(KERN_INFO "init kthread failure\n");
	kthread_release();
	return err;
}

static void kthread_exit(void) {
	kthread_release();
	printk(KERN_INFO "exit kthread\n");
}

module_init(kthread_init);
module_exit(kthread_exit);

