#include <sys/syscall.h>
#include <linux/unistd.h>
#include <stdio.h>
#include <errno.h>

#define __NR_topuser 341

long topuser(void) {
	return 0;
}

int main()
{
	return 0;
}

