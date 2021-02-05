#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define __NR_kernelps 343
#define TASK_COMM_LEN 16

long kernelps(size_t* processes_no, char** table_entries)
{
	return syscall(__NR_kernelps, processes_no, table_entries);
}

int main(int argc, char **argv)
{
	size_t processes_no;
	char (*entries_buf)[TASK_COMM_LEN];
	char **entries_pointers;
	long input_number;
	// size_t i;

	if (argc < 2) {
		if (kernelps(&processes_no, NULL)) {
			printf("Error in kernelps.\n");
			return 1;
		}

		printf("There are %zu processes running at the moment\n", 
															processes_no);
	} else {
		input_number = atol(argv[1]);
		if (input_number <= 0) {
			printf("Table size cannot be less than 1.\n");
			return 1;
		}

		processes_no = input_number;

		entries_buf = malloc(processes_no * TASK_COMM_LEN);
		if (!entries_buf) {
			printf("Error in malloc of entries buffer.\n");
			return 1;
		}

		entries_pointers = malloc(processes_no * sizeof(char *));
		if (!entries_pointers) {
			printf("Error in malloc of entries pointers.\n");
			return 1;
		}

		for (int i = 0; i < processes_no; i++) {
			entries_pointers[i] = entries_buf[processes_no - 1 - i];
		}

		if (kernelps(&processes_no, entries_pointers)) {
			printf("Error in kernelps.\n");
			return 1;
		}

		printf("List of processes' names:\n");
		for (int i = 0; i < processes_no; i++) {
			printf("'%s'\n", entries_pointers[i]);
		}
		printf("Number of processes: %zu\n", processes_no);

		free(entries_pointers);
		free(entries_buf);
	}

	return 0;
}

