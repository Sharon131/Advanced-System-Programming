#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
// #include <string.h>

#define __NR_pidtoname 345
#define TASK_COMM_LEN 16

long pidtoname(pid_t pid, char buff[TASK_COMM_LEN])
{
	return syscall(__NR_pidtoname, pid, buff);
}

int main(int argc, char **argv)
{
	char name[TASK_COMM_LEN];
	long pid;

	if (argc > 1) {
		pid = atol(argv[1]);
	} else {
		pid = getpid();
	}

	if (pid != (pid_t) pid) {
        printf("Pid value is too big.\n");
        return 1;
    }

	printf("Checking name of process with pid %ld...\n", pid);

	if (pidtoname(pid, name)) {
        printf("There is no process with that pid.\n");
        return 1;
    }

	printf("Process with pid %ld is named '%s'.\n", pid, name);

	return 0;
}
