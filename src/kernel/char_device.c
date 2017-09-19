/*
 * char_device.c
 *
 *  Created on: 2016. 10. 18.
 *      Author: root
 */



// kernel object struct header
// include/linux/kobject.h
struct kobject {
    const char      *name;
    struct list_head    entry;
    struct kobject      *parent;
    struct kset     *kset;
    struct kobj_type    *ktype;
    struct kernfs_node  *sd; /* sysfs directory entry */
    struct kref     kref;
#ifdef CONFIG_DEBUG_KOBJECT_RELEASE
    struct delayed_work release;
#endif
    unsigned int state_initialized:1;
    unsigned int state_in_sysfs:1;
    unsigned int state_add_uevent_sent:1;
    unsigned int state_remove_uevent_sent:1;
    unsigned int uevent_suppress:1;
};


// kernel module use reference
// include/linux/kref.h
struct kref {
    atomic_t refcount;
};


// include/linux/cdev.h
// character device driver struct header
struct cdev {
    struct kobject kobj;
    struct module *owner;
    const struct file_operations *ops; // driver operation
    struct list_head list;             // char device list
    dev_t dev;                         // device number
    unsigned int count;
};

// fs/char_dev.c

#define CHRDEV_MAJOR_HASH_SIZE  255
static struct char_device_struct {
    struct char_device_struct *next;
    unsigned int major;
    unsigned int baseminor;
    int minorct;
    char name[64];
    struct cdev *cdev;      /* will die */
} *chrdevs[CHRDEV_MAJOR_HASH_SIZE];
