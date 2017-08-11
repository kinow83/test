/*
 * spin_lock.c
 *
 *  Created on: 2017. 6. 13.
 *      Author: root
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>

spinlock_t g_this_lock;
struct task_struct *g_th_id1;
struct task_struct *g_th_id2;

static int thr_spin_lock_1(void *arg) {
	int i = 0;

	printk(KERN_ALERT "@ %s() : called\n", __FUNCTION__);

	spin_lock(&g_this_lock);
	{
		printk(KERN_ALERT "@ %s() : get locked\n", __FUNCTION__);

		for (i = 0; i < 30; i++) {
			printk(KERN_ALERT "@ %s() : loop\n", __FUNCTION__);
			ssleep(1);
		}
	}
	spin_unlock(&g_this_lock);

	printk(KERN_ALERT "@ %s() : unlocked\n", __FUNCTION__);
	return 0;
}

static int thr_spin_lock_2(void *arg) {
	printk(KERN_ALERT "@ %s() : called\n", __FUNCTION__);

	spin_lock(&g_this_lock);
	{
		printk(KERN_ALERT "@ %s() : get locked\n", __FUNCTION__);
		ssleep(10);
	}
	spin_unlock(&g_this_lock);

	printk(KERN_ALERT "@ %s() : unlocked\n", __FUNCTION__);
	return 0;
}

static void only_spin_lock_init(void)
{
	spin_lock_init(&g_this_lock);

	g_th_id1 = kthread_run(thr_spin_lock_1, NULL, "LOCK1");
	g_th_id2 = kthread_run(thr_spin_lock_2, NULL, "LOCK2");

	return;
}
