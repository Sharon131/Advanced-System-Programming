#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
// #include <errno.h>
// #include <err.h>

#define __NR_pidtoname 345
#define TASK_COMM_LEN 16

long pidtoname(pid_t pid, char name[TASK_COMM_LEN])
{
	return syscall(__NR_pidtoname, pid, name);
}

int main(int argc, char **argv)
{
	char name[TASK_COMM_LEN];
	long pid;

	pid = argc > 1 ? atol(argv[1]) : getpid();

	if (pid != (pid_t) pid) {
		// errx(-1, "pid value out of range");
        printf("pid value out of range");
        return -1;
    }

	printf("Checking name of process with pid %ld\n", pid);

	if (pidtoname(pid, name)) {
		// err(-1, "pidtoname");
        printf("pid value out of range");
        return -1;
    }

	printf("Process '%s' is the owner of pid %ld\n", name, pid);

	return 0;
}
