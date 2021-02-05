#include <sys/syscall.h>
#include <linux/unistd.h>
#include <stdio.h>
#include <errno.h>

#define __NR_kernelps 343

long kernelps(void) {
	return 0;
}

int main()
{
	return 0;
}

