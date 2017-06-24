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


static struct task_struct *tid[] = {
		NULL,
		NULL,
};

static int thread_fn(void *data) {
	int pid = 0;

	if (!data) {
		printk(KERN_ALERT "data is NULL\n");
	} else {
	    pid = *(int *)data;
    }

	while (!kthread_should_stop()) {
#if 0
		current->state = TASK_INTERRUPTIBLE;
#else
        set_current_state(TASK_INTERRUPTIBLE);
#endif
		schedule_timeout(HZ);
		printk(KERN_INFO "kinow: kthread running [%u]: %s\n",
				pid, __FUNCTION__);
	}
	return 0;
}

static void kthread_release(void) {
	int i;
    
	for (i=0; i<sizeof(tid)/sizeof(struct task_struct *); i++) {
        printk("kthread stop ? %d\n", task_is_stopped(tid[i]));
    }

	for (i=0; i<sizeof(tid)/sizeof(struct task_struct *); i++) {
        if (tid[i] && !task_is_stopped(tid[i])) {
		    kthread_stop(tid[i]);
        }
	}
}

static int kthread_init(void) {
	int i;
	int err = -EINVAL;

	for (i=0; i<sizeof(tid)/sizeof(struct task_struct *); i++) {
		tid[i] = kthread_run(thread_fn, NULL, "counter");
		printk("create pid = %u\n", tid[i]->pid);
		if (IS_ERR(tid[i])) {
			goto out;
		}
	}
	printk(KERN_INFO "init kthread success\n");
	return 0;
out:
    kthread_release();
	printk(KERN_INFO "init kthread failure\n");
	return err;
}

static void kthread_exit(void) {
    kthread_release();
	printk(KERN_INFO "exit kthread\n");
}

module_init(kthread_init);
module_exit(kthread_exit);

