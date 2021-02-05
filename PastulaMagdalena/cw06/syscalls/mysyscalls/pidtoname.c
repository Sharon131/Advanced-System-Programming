#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>

asmlinkage long sys_pidtoname(pid_t pid, char __user * user_buff)
{
    struct task_struct *task;
	char name[TASK_COMM_LEN];

	rcu_read_lock();
	task = find_task_by_vpid(pid);
	if (!task)
		return ESRCH;

	get_task_comm(name, task);
	rcu_read_unlock();

	if (copy_to_user(user_buff, name, TASK_COMM_LEN) != 0) {
		return EFAULT;
	}

	return 0;
}