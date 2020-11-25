#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>

MODULE_LICENSE("GPL");

#define BUFFER_SIZE     40
#define CIRCULAR_MAJOR  199

ssize_t circular_write(struct file *filp, const char __user *user_buf,
	size_t size, loff_t *f_pos);
ssize_t circular_read(struct file *filp, char __user *user_buf,
	size_t count, loff_t *f_pos);

static char *buffer;
static int write_pos; 

const struct file_operations circular_fops = {
	.read = circular_read,
	.write = circular_write,
};

static miscdevice circ_device = {
	// .minor = MISC_DYNAMIC_MINOR, // ??
	.name = "circular_module",
	.fops = &circular_file_ops,
	.mode = 00666
};

static int __init circular_init(void)
{
    /* Register a device with the given major number */
	
	int result;
    buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    
    if (!buffer) {
		result = -ENOMEM;
		goto err;
	} 
	
	buffer[0] = '\0';
	result = 0;
	// printk(KERN_INFO "The cicular module has been inserted.\n");
	
	if (misc_register(&circ_device))
	{
		printk(KERN_WARNING, "Cannot register the /dev/simple device with major number: %d\n",
			CIRCULAR_MAJOR);
		goto err;
	}

	return result;

err:
	// if (proc_entry) {
	// 	proc_remove(proc_entry);
	// }
	// unregister_chrdev(BUFFER_MAJOR, "circular");
	kfree(buffer);
	// misc_deregister(&circ_device);
	return result;
}

static void __exit circular_exit(void)
{
    /* Unregister the device and /proc entry */
	// unregister_chrdev(CIRCULAR_MAJOR, "circular");

	/* Free the buffer. No need to check for NULL - read kfree docs */
	kfree(buffer);

	printk(KERN_INFO "The cicular module has been removed\n");
}

ssize_t circular_write(struct file *filp, const char __user *user_buf,
	size_t size, loff_t *f_pos)
{
    int count = 0;

	for (count=0;count<size;count++)
	{
		if (copy_from_user(buffer+*f_pos, user_buf+count, 1))
		{
            printk(KERN_WARNING "CICULAR: could not copy data from user\n");
            return -EFAULT;
        }

		write_pos = (write_pos + 1) %BUFFER_SIZE;
	}

    // while (count < size)
    // {
    //     if (*f_pos + size > BUFFER_SIZE - 1) {
    //         size = BUFFFER_SIZE - 1 - *f_pos;
    //         //copy till end of file
    //     }
    //     else{
    //         size = *f_pos + count;
    //     }

    //     //copy rest or all if it is not bigger that buffer
        
    // }
	
	buffer[write_pos] = '\0';
	// *f_pos = (*f_pos + 1) %BUFFER_SIZE;
	// *f_pos += size;
	return size;
}

ssize_t circular_read(struct file *filp, char __user *user_buf,
	size_t count, loff_t *f_pos)
{
	size_t to_copy = strlen(buffer);

	// printk(KERN_WARNING "CIRCULAR: read f_pos is %lld\n", *f_pos);

	if (*f_pos >= to_copy) {
		return 0;
	}

	if (copy_to_user(user_buf, buffer, to_copy)) {
		printk(KERN_WARNING "CIRCULAR: could not copy data to user\n");
		return -EFAULT;
	}

	*f_pos += to_copy;
	return to_copy;
}



module_init(circular_init);
module_exit(circular_exit);