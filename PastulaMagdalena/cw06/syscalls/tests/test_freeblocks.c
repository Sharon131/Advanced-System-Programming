#include <sys/syscall.h>
#include <linux/unistd.h>
#include <stdio.h>
#include <errno.h>

#define __NR_freeblocks 344

long freeblocks(void) {
	return 0;
}

int main()
{
	return 0;
}

