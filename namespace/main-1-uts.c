
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>


#define STACK_SIZE	(1024*1024)
static char stack[STACK_SIZE];

char *const args[] = {
	"/bin/bash", NULL
};

int container_routine(void *arg) {

	printf("inside container with pid: %d\n", getpid());
	sethostname("container", 10);
	execv(args[0], args);
	printf("somethings' wrong\n");
	return 1;
}

int main() {
	printf("outside container with pid: %d\n", getpid());
	pid_t cpid = clone(container_routine, stack+sizeof(stack), 
			CLONE_NEWUTS|SIGCHLD, NULL);

	waitpid(cpid, NULL, 0);
	printf("parent - container stoped!\n");
	return 0;
}

