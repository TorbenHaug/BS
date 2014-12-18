/**
 * Translate (Caeser encryption) Module for Linuxkernel.
 * There will be added two devices to your dev folder: /dev/trans0, /dev/trans1
 * trans0 will encrypt a given chararray and trans0 will decrypt it.
 */

#include "translate.h"

struct trans_dev *trans_devices;


int trans_open(struct inode *dev_file, struct file *instance){
	printk(KERN_INFO "OPEN: Translate Module wird geoeffnet\n");
	if (deviceOpen){
		printk(KERN_WARNING "OPEN: Translate Module ist bereits geoeffnet\n");
		return -EBUSY;
	}
	sprintf(msg, "Hello from trans%d!\n", iminor(dev_file));
	msg_Ptr = msg;
	deviceOpen++;
	try_module_get(THIS_MODULE);
	return 0;
}
int trans_close(struct inode *dev_file, struct file *instance){
	printk(KERN_INFO "CLOSE: Translate Module wird verlassen\n");
	deviceOpen--;
	module_put(THIS_MODULE);
	return 0;
}
ssize_t trans_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos){
	printk(KERN_ALERT "WRITE: Sorry, this operation isn't supported.\n");
	return -EINVAL;
}

ssize_t trans_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos){
	int bytes_read = 0;

	printk(KERN_INFO "READ: Start reading.");

	if (*msg_Ptr == 0){
		return 0;
	}

	while (count && *msg_Ptr) {
		put_user(*(msg_Ptr++), buf++);
		count--;
		bytes_read++;
	}

	return bytes_read;
}


static void setup_cdev(struct trans_dev *dev, int index)
{
   int err, devno = MKDEV(major, index);

   cdev_init(&dev->cdev, &fops);
   dev->cdev.owner = THIS_MODULE;
   dev->cdev.ops = &fops;
   err = cdev_add (&dev->cdev, devno, 1);
   /* Fail gracefully if need be */
   if (err)
      printk(KERN_NOTICE "Error %d adding scull%d", err, index);
}

static int __init translate_init(void){
	printk(KERN_INFO "INIT: Translate Module wird hinzugefuegt\n");
	int i;
	int err =  alloc_chrdev_region(&trans_device, 0, 1, DEVICE_NAME);

	if (err != 0) {
	  printk(KERN_ALERT "INIT: Erstellen des devices ist fehlgeschlagen mit Error: %d\n", major);
	  return -1;
	}
	major = MAJOR(trans_device);

	trans_devices = kmalloc(2 * sizeof(struct trans_dev), GFP_KERNEL);
	if (!trans_devices) {
		translate_exit();
		return -ENOMEM;
	}
	memset(trans_devices, 0, 2 * sizeof(struct trans_dev));

	for (i = 0; i < 2; i++) {
		setup_cdev(&trans_devices[i], i);
	}
	return 0;
}

static void translate_exit(void){
	printk(KERN_INFO "EXIT: Translate Module wird entfernt\n");
	int i;

	if (trans_devices) {
		for (i = 0; i < 2; i++) {
			cdev_del(&trans_devices[i].cdev);
		}
		kfree(trans_devices);
	}

	unregister_chrdev_region(trans_device, 1);
}


module_init(translate_init);
module_exit(translate_exit);

//Metainformationen
MODULE_AUTHOR("Tim Hartig, Tim Hagemann, Torben Haug");
MODULE_LICENSE("GPL");

