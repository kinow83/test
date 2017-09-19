/*
 * interupt.c
 *
 *  Created on: 2016. 10. 6.
 *      Author: root
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/io.h>

#if 0
struct workqueue_struct {
    struct list_head    pwqs;       /* WR: all pwqs of this wq */
    struct list_head    list;       /* PR: list of all workqueues */

    struct mutex        mutex;      /* protects this wq */
    int         work_color; /* WQ: current work color */
    int         flush_color;    /* WQ: current flush color */
    atomic_t        nr_pwqs_to_flush; /* flush in progress */
    struct wq_flusher   *first_flusher; /* WQ: first flusher */
    struct list_head    flusher_queue;  /* WQ: flush waiters */
    struct list_head    flusher_overflow; /* WQ: flush overflow list */

    struct list_head    maydays;    /* MD: pwqs requesting rescue */
    struct worker       *rescuer;   /* I: rescue worker */

    int         nr_drainers;    /* WQ: drain in progress */
    int         saved_max_active; /* WQ: saved pwq max_active */

    struct workqueue_attrs  *unbound_attrs; /* PW: only for unbound wqs */
    struct pool_workqueue   *dfl_pwq;   /* PW: only for unbound wqs */

#ifdef CONFIG_SYSFS
    struct wq_device    *wq_dev;    /* I: for sysfs interface */
#endif
#ifdef CONFIG_LOCKDEP
    struct lockdep_map  lockdep_map;
#endif
    char            name[WQ_NAME_LEN]; /* I: workqueue name */

    /*
     * Destruction of workqueue_struct is sched-RCU protected to allow
     * walking the workqueues list without grabbing wq_pool_mutex.
     * This is used to dump all workqueues from sysrq.
     */
    struct rcu_head     rcu;

    /* hot fields used during command issue, aligned to cacheline */
    unsigned int        flags ____cacheline_aligned; /* WQ: WQ_* flags */
    struct pool_workqueue __percpu *cpu_pwqs; /* I: per-cpu pwqs */
    struct pool_workqueue __rcu *numa_pwq_tbl[]; /* PWR: unbound pwqs indexed by node */
};
#endif

#define MY_WORK_QUEUE_NAME "WQsched.c"

static struct workqueue_struct *my_workqueue;

static void show_irq(struct irq_desc *irq)
{
	printk(KERN_INFO "[irq desc]\n");
	printk(KERN_INFO "name: %s\n", irq->name);
	printk(KERN_INFO "parent_irq: %d\n", irq->parent_irq);
}

irqreturn_t irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	static int initialised = 0;
	static unsigned char scancode;
	static struct work_struct task;
	unsigned char status;

	// 포트에 실질적으로 값을 쓰고/읽는 기능을 주는 함수 inb()/outb()
	// 원하는 포트(port)에서 한 바이트(8bit)를 읽기 위해서는 inb(port), 쓰기 위해서는 outb(value, port)
#if 0
	]# cat /proc/ioports
	0000-0cf7 : PCI Bus 0000:00
	  0000-001f : dma1
	  0020-0021 : pic1
	  0040-0043 : timer0
	  0050-0053 : timer1
	  0060-0060 : keyboard
	  0064-0064 : keyboard
	  :
	  :
#endif
	status   = inb(0x64);
	scancode = inb(0x60);
	if (initialised == 0) {
		INIT_WORK(&task, got_char, &scancode);
	} else {

	}

}

int init_module()
{
	show_irq(irq_to_desc(0));
	show_irq(irq_to_desc(1));

	struct irq_desc *desc = irq_to_desc(1);
	my_workqueue = create_workqueue(MY_WORK_QUEUE_NAME);

	free_irq(1, NULL);

	// Request IRQ 1, the keyboard IRQ, to go to our irq_handler.
	// SA_SHIRQ means we're willing to have other handlers on this IRQ.
	// SA_INTERRUPT can be used to make the handler into a fast interrupt.
	return request_irq(1, /* The number of the keyboard IRQ on PCs */
			irq_handler, /* our handler */
			IRQF_SHARED,
			"test_keyboard_irq_handler",

			)
}

void cleanup_module()
{

}

MODULE_LICENSE("GPL");
