#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/statfs.h>

asmlinkage long sys_freeblocks(char __user* path, u64 * count) {
    struct kstatfs stat;
	int returned_value;
	u64 to_return;

	returned_value = user_statfs(path, &stat);
	if (returned_value)
    {
        return to_return;
    }

	to_return = stat.f_bfree;

    if (copy_to_user(count, &to_return, sizeof(u64)) != 0) {
        return EFAULT;
    }

	return 0;
}
