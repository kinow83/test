/*
 * chardev.h
 *
 *  Created on: 2016. 10. 5.
 *      Author: root
 */

#ifndef KERNEL_CHARDEV_H_
#define KERNEL_CHARDEV_H_

#include <linux/ioctl.h>

#define MAJOR_NUM 100


// _IOX...
//                         type       nr  size
// type: magic number
// nr: sequence number in order
// size: size of data


// Set the message of the device driver
#define IOCTL_SET_MSG _IOR(MAJOR_NUM, 0, char *)

// Get the message of the device driver
#define IOCTL_GET_MSG _IOR(MAJOR_NUM, 1, char *)

// Get the n'th byte of the message
#define IOCTL_GET_NTH_BYTE _IOWR(MAJOR_NUM, 2, int)


#define DEVICE_NAME "char_dev"

#endif /* KERNEL_CHARDEV_H_ */
