#include <sys/syscall.h>
#include <linux/unistd.h>
#include <stdio.h>
#include <errno.h>

#define __NR_timeuser 342

long timeuser(void) {
	return 0;
}

int main()
{
	return 0;
}

