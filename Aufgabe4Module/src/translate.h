/*
 * translate.h
 *
 *  Created on: 06.12.2014
 *      Author: torbenhaug
 */

#ifndef SRC_TRANSLATE_H_
#define SRC_TRANSLATE_H_
	#include <linux/kernel.h>
	#include <linux/version.h>
	#include <linux/module.h>
	#include <linux/moduleparam.h>
	#include <linux/init.h>
	#include <linux/fs.h>
	#include <linux/cdev.h>
	#include <linux/device.h>
	#include <linux/slab.h>
	#include <linux/uaccess.h>
	#include <linux/sched.h>
	#include <linux/mutex.h>


	#define NELEMS(x)  ((sizeof(x) / sizeof(x[0])-1))
	#define DEVICE_NAME "trans"
	#define BUF_LEN 40
	#define SHIFT 3
	#define READ_POSSIBLE (dev->count > 0)
	#define WRITE_POSSIBLE (dev->count < buf_len)

 	// A table of chars, which are being shifted by caesar encryption
	const char shift_table[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz"};
	int buf_len = BUF_LEN;
	int shift_size = SHIFT;

	static int major;
	static dev_t trans_device;

	static struct trans_dev {
		char data[BUF_LEN];
		char *p_in;
		char *p_out;
		int shift;
		int count;
		int readOpened;
		int writeOpened;
		wait_queue_head_t read_queue;
		wait_queue_head_t write_queue;
		struct semaphore sem;
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
	    .read =     trans_read,
	    .write =    trans_write,
	    .open =     trans_open,
	    .release =  trans_close,
	};
#endif /* SRC_TRANSLATE_H_ */
