/*
 * translate.h
 *
 *  Created on: 06.12.2014
 *      Author: torbenhaug
 */

#ifndef SRC_TRANSLATE_H_
#define SRC_TRANSLATE_H_
	#include <linux/version.h>
	#include <linux/module.h>
	#include <linux/init.h>
	#include <linux/fs.h>
	#include <linux/cdev.h>
	#include <linux/device.h>
	#include <linux/slab.h>
	#include <linux/uaccess.h>

	#define DEVICE_NAME "trans"
	#define BUF_LEN 80

	static int major;
	static dev_t trans_device;
	static int deviceOpen = 0;
	static char msg[BUF_LEN];
	static char *msg_Ptr;

	static struct trans_dev{
		struct cdev cdev;
	};

	static int __init translate_init(void);
	static void translate_exit(void);

	int trans_open(struct inode *dev_file, struct file *instance);
	int trans_close(struct inode *dev_file, struct file *instance);
	ssize_t trans_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
	ssize_t trans_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
	//int trans_ioctl(struct inode *inode, struct file *file, unsigned int ioctl_num, unsigned long ioctl_param);



	static struct file_operations fops = {
		.owner =    THIS_MODULE,
		//.ioctl = 	trans_ioctl,
	    .read =     trans_read,
	    .write =    trans_write,
	    .open =     trans_open,
	    .release =  trans_close,
	};
#endif /* SRC_TRANSLATE_H_ */
