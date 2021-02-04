#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>

asmlinkage long sys_pidtoname(pid_t pid, char __user * user_buff)
{
    struct task_struct *task;
	char comm[TASK_COMM_LEN];

	rcu_read_lock();
	task = find_task_by_vpid(pid);
	if (!task)
		return -ESRCH;

	get_task_comm(comm, task);
	rcu_read_unlock();

	return copy_to_user(user_buff, comm, TASK_COMM_LEN) ? -EFAULT : 0;
}