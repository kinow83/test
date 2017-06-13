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

struct task_struct *g_th_id = NULL;

static int kthread_example_thr_fun(void *arg) {
	printk(KERN_ALERT "@ %s() : called\n", __FUNCTION__);
	while (!kthread_should_stop()) {
		printk(KERN_ALERT "@ %s() : loop\n", __FUNCTION__);
		ssleep(1);
	}
	printk(KERN_ALERT "@ %s() : kthread_should_stop() called. Bye.\n",
			__FUNCTION__);
	return 0;
}

static int kthread_example_init(void)
{
  printk(KERN_ALERT "@ %s() : called\n", __FUNCTION__);

  if(g_th_id == NULL){
     g_th_id = (struct task_struct *)kthread_run(kthread_example_thr_fun, NULL, "kthread_example");
}
  return 0;
}

static void kthread_example_release(void)
{
  if(g_th_id){
     kthread_stop(g_th_id);
     g_th_id = NULL;
  }
  printk(KERN_ALERT "@ %s() : Bye.\n", __FUNCTION__);
}

module_init(kthread_example_init);
module_exit(kthread_example_release);
MODULE_LICENSE("Dual BSD/GPL");


