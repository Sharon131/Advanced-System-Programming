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

ssize_t circular_write(struct file *filp, const char __user *user_buf,
	size_t size, loff_t *f_pos);
ssize_t circular_read(struct file *filp, char __user *user_buf,
	size_t count, loff_t *f_pos);
ssize_t circular_write_proc(struct file *filp, const char __user *user_buf,
	size_t size, loff_t *f_pos);

static char *buffer;
static unsigned long long buffer_curr_size = BUFFER_SIZE;
static int write_pos;

struct proc_dir_entry *proc_entry;

const struct file_operations circular_fops = {
	.read = circular_read,
	.write = circular_write,
};

static struct miscdevice circ_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "circular",
	.fops = &circular_fops,
	.mode = 00666
};

static struct file_operations proc_fops = {
	.write = circular_write_proc,
};

static int __init circular_init(void)
{
	int result;
    proc_entry = proc_create("circular", 0000, NULL, &proc_fops);
	if (!proc_entry) {
		printk(KERN_WARNING "Cannot create /proc/circular\n");
		goto err;
	}

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
		printk(KERN_WARNING, "Cannot register the /dev/circular device.\n");
		goto err;
	}

	return result;

err:
	if (proc_entry) {
		proc_remove(proc_entry);
	}
	kfree(buffer);
	misc_deregister(&circ_device);
	return result;
}

static void __exit circular_exit(void)
{
	/* Free the buffer. No need to check for NULL - read kfree docs */
	proc_remove(proc_entry);
	kfree(buffer);
	misc_deregister(&circ_device);

	printk(KERN_INFO "The cicular module has been removed\n");
}

ssize_t circular_write(struct file *filp, const char __user *user_buf,
	size_t size, loff_t *f_pos)
{
    int count = 0;

	for (count=0;count<size;count++)
	{
		if (copy_from_user(buffer+write_pos, user_buf+count, 1))
		{
            printk(KERN_WARNING "CICULAR: could not copy data from user\n");
            return -EFAULT;
        }

		write_pos = (write_pos + 1) %BUFFER_SIZE;
	}

	
	buffer[write_pos] = '\0';
	// *f_pos = (*f_pos + 1) %BUFFER_SIZE;
	// *f_pos += size;
	return size;
}

ssize_t circular_read(struct file *filp, char __user *user_buf,
	size_t count, loff_t *f_pos)
{
	size_t to_copy = strlen(buffer);

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

ssize_t circular_write_proc(struct file *filp, const char __user *user_buf,
	size_t size, loff_t *f_pos)
{
	char* new_buffer;
	unsigned long long buffer_new_size;

	if (kstrtoull_from_user(user_buf, size, 10, &buffer_new_size))
	{
		printk(KERN_WARNING "CICULAR: could not copy data from user\n");
		return -EFAULT;	
	}

	if (buffer_new_size == buffer_curr_size)
	{
		return size;
	}

	if (buffer_new_size == 0)
	{
		printk(KERN_WARNING "CICULAR: buffer size must be greater than zero\n");
		return -EFAULT;	
	}

	new_buffer = kmalloc(buffer_new_size, GFP_KERNEL);

	if (!new_buffer)
	{
		printk(KERN_WARNING "CICULAR: could not allocate\n");
		return -EFAULT;	
	}

	if (buffer_new_size >= buffer_curr_size)
	{
		memcpy(new_buffer, buffer, buffer_curr_size);
	}
	else
	{
		memcpy(new_buffer, buffer, buffer_new_size-1);
		new_buffer[buffer_new_size-1] = 0;
	}

	kfree(buffer);
	buffer = new_buffer;
	buffer_curr_size = buffer_new_size;

	return size;
}
module_init(circular_init);
module_exit(circular_exit);

