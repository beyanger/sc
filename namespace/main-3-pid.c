
#define _GNU_SOURCE
#include <stdlib.h>
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

#define CLONE_FLAGS		(CLONE_NEWPID|CLONE_NEWUTS|CLONE_NEWIPC|SIGCHLD)

/*
 *	使用了  CLONE_NEWPID 子进程会进入一个新的PID空间，由于top/ps等命令
 *	通过读取proc文件系统来显示，在子进程中使用top/ps命令仍然可以看到系统进程，
 *	使用 mount -t proc proc /proc 之后则仅能看到子进程的pid
 *	退出子进程后，需要重新挂载父进程的proc文件系统
 * */

int container_routine(void *arg) {

	printf("inside container with pid: %d\n", getpid());
	sethostname("container", 10);
	system("mount -t proc proc /proc");
	execv(args[0], args);
	printf("somethings' wrong\n");
	return 1;
}

int main() {
	printf("outside container with pid: %d\n", getpid());
	pid_t cpid = clone(container_routine, stack+sizeof(stack), 
			CLONE_FLAGS, NULL);

	waitpid(cpid, NULL, 0);
	printf("parent - container stoped!\n");
	return 0;
}

