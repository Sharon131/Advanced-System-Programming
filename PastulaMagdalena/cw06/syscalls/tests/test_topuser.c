#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>

#define __NR_topuser 341

long topuser(uid_t* user_id) 
{
	return syscall(__NR_topuser, user_id);
}

int main()
{
	uid_t top_uid;

	if (topuser(&top_uid)) {
		printf("Error while getting top user.");
		return 1;
	}

	printf("User with id %ld has the most processes at the moment\n", (long) top_uid);
}

