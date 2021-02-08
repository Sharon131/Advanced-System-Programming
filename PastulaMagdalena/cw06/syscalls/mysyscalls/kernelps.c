#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/slab.h>

static long count_processes(size_t __user* entries_no)
{
	struct task_struct *task;
	size_t processes_no = 0;

	read_lock(&tasklist_lock);
	for_each_process(task) {
        processes_no++;
    }
	read_unlock(&tasklist_lock);

    if (copy_to_user(entries_no, &processes_no, sizeof(size_t)) != 0) {
        return EFAULT;
    }

	return 0;
}

static long list_processes(size_t __user* entries_no, 
                            char __user* __user* entries_table)
{
	int code = 0;
	size_t entries_no_value;
	struct task_struct *task;
	size_t processes_no = 0;
	char (*proc_names_buf)[TASK_COMM_LEN];
	char __user* entry;
    unsigned int i;

	if (copy_from_user(&entries_no_value, entries_no, sizeof(size_t))) {
		return EFAULT;
    }

	proc_names_buf = kmalloc(TASK_COMM_LEN * entries_no_value, GFP_KERNEL);
	if (!proc_names_buf) {
		return ENOMEM;
    }

	read_lock(&tasklist_lock);
	for_each_process(task) {
		if (++processes_no > entries_no_value) {
			code = EFAULT;
            // read_unlock(&tasklist_lock);
        }

		if (!code) {
			get_task_comm(proc_names_buf[processes_no - 1], task);
        }
	}
	read_unlock(&tasklist_lock);

	if (copy_to_user(entries_no, &processes_no, sizeof(size_t))) {
		code = EFAULT;
    }

	if (code) {
		goto out;
    }

	for (i = 0; i < processes_no; i++) {
		if (copy_from_user(&entry, &entries_table[i], sizeof(char *)) ||
		    copy_to_user(entry, proc_names_buf[i], TASK_COMM_LEN)) {
			code = EFAULT;
			goto out;
		}
	}

out:
	kfree(proc_names_buf);
	return code;
}

asmlinkage long sys_kernelps(size_t __user * entries_no,
		char __user *__user * entries_table) 
{     
    if (entries_table == NULL) {
        return count_processes(entries_no);
    } else {
        return list_processes(entries_no, entries_table);
    }
}
