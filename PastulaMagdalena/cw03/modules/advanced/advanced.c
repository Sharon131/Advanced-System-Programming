#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <asm/param.h>

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/namei.h>
#include <linux/seq_file.h>
#include <linux/mount.h>
// #include <linux/sched/task.h>

MODULE_LICENSE("GPL");

#define JIFFIES_BUFF_SIZE   50
#define MOUNT_PATH_SIZE     100

char jiffies_buffer[50];

char process_name[TASK_COMM_LEN];
uint16_t process_name_len = 0;

char mount_path[MOUNT_PATH_SIZE];
uint16_t mount_path_len = 0;

ssize_t prname_write(struct file *filp, const char __user *user_buf,
	size_t size, loff_t *f_pos);
ssize_t prname_read(struct file *filp, char __user *user_buf,
	size_t count, loff_t *f_pos);

ssize_t jiffies_read(struct file *filp, char __user *user_buf,
	size_t count, loff_t *f_pos);

ssize_t mounderef_write(struct file *filp, const char __user *user_buf,
	size_t size, loff_t *f_pos);
ssize_t mountderef_read(struct file *filp, char __user *user_buf,
	size_t count, loff_t *f_pos);


const struct file_operations prname_fops = {
	.write = prname_write,
	.read = prname_read,
};

const struct file_operations jiffies_fops = {
	.read = jiffies_read,
};

const struct file_operations mountderef_fops = {
	.write = mounderef_write,
    .read = mountderef_read,
};

static struct miscdevice prname_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "prname",
	.fops = &prname_fops,
	.mode = 00666
};

static struct miscdevice jiffies_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "jiffies",
	.fops = &jiffies_fops,
	.mode = 00444,
};

static struct miscdevice mountderef_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mountderef",
	.fops = &mountderef_fops,
	.mode = 00666
};

static int __init advanced_init(void)
{
	int result = 0;

	if (misc_register(&prname_device))
	{
		result = -ENOMEM;
		printk(KERN_WARNING "Cannot register the /dev/prname device.\n");
		goto err;
	}
	if (misc_register(&jiffies_device))
	{
		result = -ENOMEM;
		printk(KERN_WARNING "Cannot register the /dev/prname device.\n");
		goto err;
	}
	if (misc_register(&mountderef_device))
	{
		result = -ENOMEM;
		printk(KERN_WARNING "Cannot register the /dev/prname device.\n");
		goto err;
	}

	return result;

err:
	misc_deregister(&prname_device);
	misc_deregister(&jiffies_device);
	misc_deregister(&mountderef_device);
	return result;
}

static void __exit advanced_exit(void)
{
	misc_deregister(&prname_device);
	misc_deregister(&jiffies_device);
	misc_deregister(&mountderef_device);

	printk(KERN_INFO "The cicular module has been removed\n");
}


ssize_t prname_write(struct file *filp, const char __user *user_buf,
	size_t size, loff_t *f_pos)
{
    int rc;
	long _new_pid;
	pid_t new_pid;
	struct pid *new_pidp;
	struct task_struct *taskp;

	rc = kstrtol_from_user(user_buf, size, 10, &_new_pid);
	if (rc)
		return rc;

	if (_new_pid < 1)
		return -EINVAL;

	new_pid = _new_pid;

	new_pidp = find_get_pid(new_pid);
	if (!new_pidp)
    {
        return -ESRCH;
    }

	taskp = get_pid_task(new_pidp, PIDTYPE_PID);

	put_pid(new_pidp);

	if (!taskp)
    {
        return -ESRCH;
    }

	get_task_comm(process_name, taskp);
	put_task_struct(taskp);

	process_name_len = strlen(process_name);

	return size;
}

ssize_t prname_read(struct file *filp, char __user *user_buf,
	size_t count, loff_t *f_pos)
{
	uint16_t bytes_to_copy;
	uint32_t not_copied;

	if (process_name_len == 0)
    {
        return -EBUSY;
    }

	if (*f_pos < 0)
    {
        return -EINVAL;
    }

	if (count==0 || *f_pos >= process_name_len)
    {
        return 0;
    }

	bytes_to_copy = (process_name_len-*f_pos < count) ? process_name_len-*f_pos : count;

	not_copied = copy_to_user(user_buf, process_name + *f_pos, bytes_to_copy);

	if (not_copied)
    {
        *f_pos += bytes_to_copy - not_copied;
		return -EFAULT;
    }
        
	*f_pos += bytes_to_copy;

	return bytes_to_copy;
}

ssize_t jiffies_read(struct file *filp, char __user *user_buf,
	size_t count, loff_t *f_pos)
{
    uint16_t bytes_to_copy;
	uint8_t jiffies_len;
    uint8_t not_copied;

	if (*f_pos < 0)
    {
        return -EINVAL;
    }
		
	sprintf(jiffies_buffer, "%lu\r\n", jiffies);
	jiffies_len = strlen(jiffies_buffer);

	bytes_to_copy = (jiffies_len-*f_pos < count) ? jiffies_len-*f_pos: count;

    not_copied = copy_to_user(user_buf, jiffies_buffer, bytes_to_copy);
	if (not_copied)
    {
        *f_pos += bytes_to_copy - not_copied;
        return -EFAULT;
    }

	*f_pos += bytes_to_copy;

	return bytes_to_copy;
}

ssize_t mounderef_write(struct file *filp, const char __user *user_buf,
	size_t size, loff_t *f_pos)
{
    struct path _path, root_path;
	uint32_t rc;
	char *new_mount_path;
    char given_path[MOUNT_PATH_SIZE];

	if (copy_from_user(given_path, user_buf, size)) {
		return -EFAULT;
	}

	given_path[size] = 0;

	rc = kern_path(given_path, LOOKUP_FOLLOW, &_path);
	if (rc)
		return rc;

	dput(_path.dentry);

	root_path = (struct path) {
		.dentry = dget(_path.mnt->mnt_root),
		.mnt = _path.mnt
	};

	new_mount_path = d_path(&root_path, given_path, size + 1);
    new_mount_path[size] = 0;

	path_put(&root_path);

	if (IS_ERR(new_mount_path)) {
		rc = PTR_ERR(new_mount_path);
        return rc;
	}

    strcpy(mount_path, new_mount_path);
	mount_path_len = strlen(mount_path);

	return size;
}

ssize_t mountderef_read(struct file *filp, char __user *user_buf,
	size_t count, loff_t *f_pos)
{
    size_t bytes_to_copy;
	uint64_t not_copied;

	if (mount_path_len == 0) {
        return -EBUSY;
    }

	if (*f_pos < 0) {
        return -EINVAL;
    }

	if (count==0 || *f_pos >= mount_path_len) {
        return 0;
    }

	bytes_to_copy = mount_path_len - *f_pos < count ? (mount_path_len - *f_pos) : count;

	not_copied = copy_to_user(user_buf, mount_path + *f_pos, bytes_to_copy);

	if (not_copied) {
        return -EFAULT;
    }

	*f_pos += bytes_to_copy;

	return bytes_to_copy;
}


module_init(advanced_init);
module_exit(advanced_exit);

