/**
 * Translate (Caeser encryption) Module for Linuxkernel.
 * There will be added two devices to your dev folder: /dev/trans0, /dev/trans1
 * trans0 will encrypt a given chararray and trans0 will decrypt it.
 */

#include "translate.h"

struct trans_dev *trans_devices;

int find_index(const char a[],const int size, const char value)
{
   int i = 0;
   for (i=0; i<size; i++)
   {
	 if (a[i] == value)
	 {
	    return i;
	 }
   }
   return(-1);
}

char caeser(char move_char, int shift){
	int index = find_index(shift_table, NELEMS(shift_table), move_char);
	if(index >= 0){

		int new_index = (index + shift);
		while (new_index<0){
			new_index = new_index + NELEMS(shift_table);
		}
		new_index %= NELEMS(shift_table);
		//printf("%d:%d:%d",index,new_index,shift);
		move_char = shift_table[new_index];
	}
	return move_char;
}

int trans_open(struct inode *dev_file, struct file *instance){
	struct trans_dev *dev;
	printk(KERN_INFO "OPEN: Translate Module wird geoeffnet\n");
	dev = container_of(dev_file->i_cdev, struct trans_dev, cdev);
	instance->private_data = dev;
	if((instance->f_flags & O_ACCMODE) == O_WRONLY && !dev->writeOpened){
		dev->writeOpened = 1;
	}else if((instance->f_flags & O_ACCMODE) == O_RDONLY && !dev->readOpened){
		dev->readOpened = 1;
	}else if((instance->f_flags & O_ACCMODE) == O_RDWR && !dev->readOpened && !dev->writeOpened){
		dev->writeOpened = 1;
		dev->readOpened = 1;
	}
	else{
		printk(KERN_WARNING "OPEN: Translate Module ist bereits geoeffnet\n");
		return -EBUSY;
	}

	try_module_get(THIS_MODULE);
	return 0;
}
int trans_close(struct inode *dev_file, struct file *instance){
	printk(KERN_INFO "CLOSE: Translate Module wird verlassen\n");
	struct trans_dev *dev;
	dev = container_of(dev_file->i_cdev, struct trans_dev, cdev);
	instance->private_data = dev;
	if((instance->f_flags & O_ACCMODE) == O_WRONLY && dev->writeOpened){
		dev->writeOpened = 0;
	}else if((instance->f_flags & O_ACCMODE) == O_RDONLY && dev->readOpened){
		dev->readOpened = 0;
	}else if((instance->f_flags & O_ACCMODE) == O_RDWR && dev->readOpened && dev->writeOpened){
		dev->writeOpened = 0;
		dev->readOpened = 0;
	}
	module_put(THIS_MODULE);
	return 0;
}
ssize_t trans_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos){
	printk(KERN_INFO "WRITE: Starte schreiben von %d Zeichen.\n", count);
	struct trans_dev *dev = filp->private_data;
	int bytes_written = 0;
	for(bytes_written = 0; (bytes_written < count); bytes_written++){
		char tmp = *buf;
		get_user(*(dev->p_in), buf++);
		*dev->p_in = caeser(*dev->p_in,dev->shift);
		dev->p_in++;
		dev->count++;

		printk(KERN_INFO "WRITE: schreibe zeichen %d\n", bytes_written + 1);
		if(((dev->p_in - dev->data) % BUF_LEN) == 0){
			dev->p_in = dev->data;
		}
		wake_up_interruptible(&dev->read_queue);
		while(!WRITE_POSSIBLE){
			if(filp->f_flags&O_NONBLOCK){
				return -EAGAIN;
			}
			if(wait_event_interruptible(dev->write_queue,WRITE_POSSIBLE)){
				return -ERESTARTSYS;
			}
		}
	}
	printk(KERN_INFO "WRITE: %d konnten nicht geschrieben werden\n", count - bytes_written);
	return bytes_written;
}

ssize_t trans_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos){
	int bytes_read = 0;
	struct trans_dev *dev = filp->private_data;
	printk(KERN_INFO "READ: Start reading.");

	while (count && dev->count > 0) {
		put_user(*(dev->p_out++), buf++);
		count--;
		if(((dev->p_out - dev->data) % BUF_LEN) == 0){
			dev->p_out = dev->data;
		}
		dev->count--;
		bytes_read++;
		wake_up_interruptible(&dev->write_queue);
		/*while(!READ_POSSIBLE){
			if(filp->f_flags&O_NONBLOCK){
				return -EAGAIN;
			}
			if(wait_event_interruptible(dev->read_queue,READ_POSSIBLE)){
				return -ERESTARTSYS;
			}
		}*/

	}

	return bytes_read;
}


static void setup_cdev(struct trans_dev *dev, int index)
{
   int err, devno = MKDEV(major, index);

   cdev_init(&dev->cdev, &fops);
   dev->cdev.owner = THIS_MODULE;
   dev->cdev.ops = &fops;
   dev->shift = (index ? -SHIFT : SHIFT);
   dev->readOpened = 0;
   dev->writeOpened = 0;
   init_waitqueue_head(&dev->read_queue);
   init_waitqueue_head(&dev->write_queue);
   err = cdev_add (&dev->cdev, devno, 1);
   /* Fail gracefully if need be */
   if (err){
      printk(KERN_NOTICE "Error %d adding scull%d", err, index);
   }
   dev->p_in = dev->data;
   dev->p_out = dev->data;
   dev->count = 0;
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
module_param(buf_len, int, S_IRUGO);
module_param(shift_size, int, S_IRUGO);
MODULE_PARM_DESC(buf_len, "Internal buffer size");
MODULE_PARM_DESC(shift_size, "Internal shift size");

