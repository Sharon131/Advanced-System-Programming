#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#define __NR_freeblocks 344

long freeblocks(char* path, uint64_t* count) {
	return syscall(__NR_freeblocks, path, count);
}

int main(int argc, char **argv)
{
	char* path;
	uint64_t freeblocks_no;

	if (argc > 1) {
		path = argv[1];
	} else {
		path = "./";
	}

	if (freeblocks(path, &freeblocks_no) != 0) {
		printf("Error while looking for freeblocks.\n");
		return 1;
	}

	printf("%llu free blocks on %s\n", (unsigned long long) freeblocks_no, 
										path);

	return 0;
}

