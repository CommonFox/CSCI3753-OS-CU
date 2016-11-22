#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<asm/uaccess.h>

#define device_name "simple_character_device"

static ssize_t char_read (struct file * file, char __user * user, size_t len, loff_t * lofft);
static ssize_t char_write (struct file * file, const char __user * user, size_t len, loff_t * lofft);
static int char_open (struct inode * inode, struct file * file);
static int char_release (struct inode * inode, struct file * file);

static int counter = 0;
static int majorNum = 0;
static char message[1000];
static short messageSize = 0;

static struct file_operations fops =
{
   .open = char_open,
   .read = char_read,
   .write = char_write,
   .release = char_release,
};

static int char_open (struct inode * inode, struct file * file)
{
	counter++;
	printk(KERN_ALERT "The device has been opened %d times\n", counter);

	return 0;
};

static int char_release (struct inode * inode, struct file * file)
{
	printk(KERN_ALERT "The device has been opened %d times\n", counter);

	return 0;
};

static ssize_t char_read (struct file * file, char __user * user, size_t len, loff_t * lofft)
{
    	messageSize = strlen(message);
    	copy_to_user(user, message, messageSize);

    	printk(KERN_ALERT "Message: %s\n", message);
    	printk(KERN_ALERT "The size of the message is %d characters\n", messageSize);
};

static ssize_t char_write (struct file * file, const char __user * user, size_t len, loff_t * lofft)
{		
	copy_from_user(message + messageSize, user, strlen(user));
	messageSize = strlen(message);

	printk(KERN_ALERT "The size of the message is %d chracters\n", strlen(user));
};

int hello_init(void)
{
	majorNum = register_chrdev(240, device_name, &fops);
	return 0;
}

int hello_exit(void)
{
	unregister_chrdev(majorNum, device_name);
	return 0;
}

module_init(hello_init);
module_exit(hello_exit);
