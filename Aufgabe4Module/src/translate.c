/**
 * Translate (Caeser encryption) Module for Linuxkernel.
 * There will be added two devices to your dev folder: /dev/trans0, /dev/trans1
 * trans0 will encrypt a given chararray and trans0 will decrypt it.
 */

#include "translate.h"

struct trans_dev *trans_devices;

/**
	Finds the index of a character in an array of characters.
*/
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

/**
	Open event when the driver gets opened
*/
int trans_open(struct inode *dev_file, struct file *instance){
	struct trans_dev *dev;
	printk(KERN_INFO "OPEN: Translate Module wird geoeffnet\n");
	// Get device handle from class device
	dev = container_of(dev_file->i_cdev, struct trans_dev, cdev);
	instance->private_data = dev;
	// Try to lock semaphore
	if (down_interruptible(&dev->sem)){
		 return -ERESTARTSYS;
	}

	// Set flags depending on what mode the driver got opened with
	// Write only mode
	if((instance->f_flags & O_ACCMODE) == O_WRONLY && !dev->writeOpened){
		dev->writeOpened = 1;
	// Read only mode
	}else if((instance->f_flags & O_ACCMODE) == O_RDONLY && !dev->readOpened){
		dev->readOpened = 1;
	// Read and write
	}else if((instance->f_flags & O_ACCMODE) == O_RDWR && !dev->readOpened && !dev->writeOpened){
		dev->writeOpened = 1;
		dev->readOpened = 1;
	}
	else{
		// Unknown mode for this module or the requested mode is already used.
		printk(KERN_WARNING "OPEN: Translate Module ist bereits geoeffnet\n");
		return -EBUSY;
	}

	// Set usage of this module
	try_module_get(THIS_MODULE);
	// Lift semaphore lock
	up(&(dev->sem));
	return 0;
}

/**
	Close event for driver file
*/
int trans_close(struct inode *dev_file, struct file *instance){
	printk(KERN_INFO "CLOSE: Translate Module wird verlassen\n");
	struct trans_dev *dev;
	// Get device handle from file
	dev = container_of(dev_file->i_cdev, struct trans_dev, cdev);
	instance->private_data = dev;
	// Reset flags depending on modes, s.a.
	if((instance->f_flags & O_ACCMODE) == O_WRONLY && dev->writeOpened){
		dev->writeOpened = 0;
	}else if((instance->f_flags & O_ACCMODE) == O_RDONLY && dev->readOpened){
		dev->readOpened = 0;
	}else if((instance->f_flags & O_ACCMODE) == O_RDWR && dev->readOpened && dev->writeOpened){
		dev->writeOpened = 0;
		dev->readOpened = 0;
	}

	// Update module usage
	module_put(THIS_MODULE);
	return 0;
}

/**
	When someone writes to the driver file
*/
ssize_t trans_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos){
	printk(KERN_INFO "WRITE: Starte schreiben von %d Zeichen.\n", count);
	// Get device from driver file
	struct trans_dev *dev = filp->private_data;
	int bytes_written = 0;
	// Writeloop for required bytes to write
	for(bytes_written = 0; (bytes_written < count); bytes_written++){
		// Check for condition variable to be sure we can write (e.g. buffer is full so we'd have to wait)
		if(!WRITE_POSSIBLE){
			// IF we should not block error out as we're forced to block
			if(filp->f_flags&O_NONBLOCK){
				return -EAGAIN;
			}
			
			// Wait for condition
			if(wait_event_interruptible(dev->write_queue,WRITE_POSSIBLE)){
				return -ERESTARTSYS;
			}
		}

		// Get current character
		char tmp = *buf;
		// Get character from userspace into kernelspace
		get_user(*(dev->p_in), buf++);
		// Shift character
		*dev->p_in = caeser(*dev->p_in,dev->shift);
		// Move to empty character
		dev->p_in++;
		// Increase count of buffered characters
		dev->count++;

		printk(KERN_INFO "WRITE: schreibe zeichen %d\n", bytes_written + 1);
		// If buffer is full, move cached data to buffer
		if(((dev->p_in - dev->data) % BUF_LEN) == 0){
			dev->p_in = dev->data;
		}
		// Notify read process to have more data to read
		wake_up_interruptible(&dev->read_queue);
	}
	printk(KERN_INFO "WRITE: %d konnten nicht geschrieben werden\n", count - bytes_written);
	// Return written bytes
	return bytes_written;
}

/**
	When someone reads from the driver file
*/
ssize_t trans_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos){
	int bytes_read = 0;
	// Get device from file handle
	struct trans_dev *dev = filp->private_data;
	printk(KERN_INFO "READ: Start reading.");

	// While there should be something to read
	while (count) {
		// If there's nothing to read however
		if(!READ_POSSIBLE){
			// Error if we shouldn't block
			if(filp->f_flags&O_NONBLOCK){
				return -EAGAIN;
			}

			// Wait for new data to be available
			if(wait_event_interruptible(dev->read_queue,READ_POSSIBLE)){
				return -ERESTARTSYS;
			}
		}

		// Put data from kernelspace into userspace
		put_user(*(dev->p_out++), buf++);
		// Decrease amount to read
		count--;
		// If there's nothing to read from cache, but from buffer
		if(((dev->p_out - dev->data) % BUF_LEN) == 0){
			dev->p_out = dev->data;
		}
		// Decrease size of held data
		dev->count--;
		bytes_read++;
		// Notify write process
		wake_up_interruptible(&dev->write_queue);
		printk(KERN_INFO "Noch zu lesende Zeichen: %d\n", count);
	}
	// Return amount of read bytes
	return bytes_read;
}

/**
	Setup of character device
*/
static void setup_cdev(struct trans_dev *dev, int index)
{
   // Sets the device number that we got from the kernel
   int err, devno = MKDEV(major, index);

   // Init driver with out file operation handles
   cdev_init(&dev->cdev, &fops);
   // Set owner of the driver to this module
   dev->cdev.owner = THIS_MODULE;
   // Set the operations again
   dev->cdev.ops = &fops;
   // Set shifting amount for caesar enc. We do - the amount if we're decrypting.
   dev->shift = (index ? -SHIFT : SHIFT);
   // Reset usages
   dev->readOpened = 0;
   dev->writeOpened = 0;
   // Init queues for events
   init_waitqueue_head(&dev->read_queue);
   init_waitqueue_head(&dev->write_queue);
   // Init semaphore
   sema_init(&(dev->sem),1);
   // Add the driver to the kernel
   err = cdev_add (&dev->cdev, devno, 1);
   /* Fail gracefully if need be */
   if (err){
      printk(KERN_NOTICE "Error %d adding scull%d", err, index);
   }
   // Set the current cache to data
   dev->p_in = dev->data;
   dev->p_out = dev->data;
   dev->count = 0;
}

/**
	Init module method
*/
static int __init translate_init(void){
	printk(KERN_INFO "INIT: Translate Module wird hinzugefuegt\n");
	int i;
	// Try and get region for drivers
	int err =  alloc_chrdev_region(&trans_device, 0, 1, DEVICE_NAME);

	if (err != 0) {
	  printk(KERN_ALERT "INIT: Erstellen des devices ist fehlgeschlagen mit Error: %d\n", major);
	  return -1;
	}
	// Get major id of our device
	major = MAJOR(trans_device);

	// Allocate memory for our devices
	trans_devices = kmalloc(2 * sizeof(struct trans_dev), GFP_KERNEL);
	if (!trans_devices) {
		// If we couldn't alloc memory in kernel -> exit
		translate_exit();
		return -ENOMEM;
	}
	// Set the allocated memory for our devices
	memset(trans_devices, 0, 2 * sizeof(struct trans_dev));

	// setup each device
	for (i = 0; i < 2; i++) {
		setup_cdev(&trans_devices[i], i);
	}
	return 0;
}

/**
	Module exit method
*/
static void translate_exit(void){
	printk(KERN_INFO "EXIT: Translate Module wird entfernt\n");
	int i;
	
	// If we previously were able to allocate our devices
	if (trans_devices) {
		// Remove each device
		for (i = 0; i < 2; i++) {
			cdev_del(&trans_devices[i].cdev);
		}
		// Free the previously allocated memory for our devices
		kfree(trans_devices);
	}

	// Unregister our region for our driver
	unregister_chrdev_region(trans_device, 1);
}

// Set the init and exit method references
module_init(translate_init);
module_exit(translate_exit);

//Metainformationen
MODULE_AUTHOR("Tim Hartig, Tim Hagemann, Torben Haug");
MODULE_LICENSE("GPL");
module_param(buf_len, int, S_IRUGO);
module_param(shift_size, int, S_IRUGO);
MODULE_PARM_DESC(buf_len, "Internal buffer size");
MODULE_PARM_DESC(shift_size, "Internal shift size");

