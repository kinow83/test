/*
 * work_queue.c
 *
 *  Created on: 2016. 10. 12.
 *      Author: root
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/param.h>

MODULE_LICENSE( "GPL" );


static struct workqueue_struct *my_wq;

typedef struct {
	struct work_struct my_work;
	int x;
} my_work_t;

typedef struct {
	struct delayed_work my_work;
	int x;
} my_delay_work_t;


my_work_t *work1, *work2;

my_delay_work_t *dwork;


static void my_wq_function ( struct work_struct * work ) {
	my_work_t *my_work = (my_work_t *)work;
	printk("my_work.x: %d\n", my_work->x);
	kfree((void*)my_work);
	return;
}

static void my_delay_wq_function ( struct work_struct * work ) {
	my_delay_work_t *my_work = (my_delay_work_t *)work;
	printk("my_delay_work.x: %d\n", my_work->x);
	kfree((void*)my_work);
	return;
}

int init_module(void)
{
	int ret;

	printk("HZ = %ld\n", HZ);
	printk("jiffies = %ld\n", jiffies);

	my_wq = create_workqueue("my_queue");
	if (my_wq) {

		// Queue some work (item 1)
		work1 = (my_work_t *)kmalloc(sizeof(my_work_t), GFP_KERNEL);
		if (work1) {
			INIT_WORK((struct work_struct *)work1, my_wq_function);
			work1->x = 1;

			ret = queue_work(my_wq, (struct work_struct *)work1);
		}

		// Queue some work (item 2)
		work2 = (my_work_t *)kmalloc(sizeof(my_work_t), GFP_KERNEL);
		if (work2) {
			INIT_WORK((struct work_struct *)work2, my_wq_function);
			work2->x = 2;

			ret = queue_work(my_wq, (struct work_struct *)work2);
		}

		dwork = (my_delay_work_t *)kmalloc(sizeof(my_delay_work_t), GFP_KERNEL);
		if (dwork) {
			INIT_DELAYED_WORK((struct delayed_work *)dwork, my_delay_wq_function);
			dwork->x = 3;

			ret = queue_delayed_work(my_wq, (struct delayed_work *)dwork,  5 * HZ);

//			cancel_delayed_work_sync((struct delayed_work *)dwork);
		}

		printk("work1 queue pendding ? %d\n", work_pending((struct work_struct *)work1));
		printk("work2 queue pendding ? %d\n", work_pending((struct work_struct *)work2));
		printk("delay work queue pendding ? %d\n", delayed_work_pending((struct delayed_work *)dwork));
	}
	return 0;
}

void cleanup_module( void )
{
	flush_workqueue(my_wq);

	destroy_workqueue(my_wq);

	return;
}
