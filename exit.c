#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
void exit_shell();

void exit_shell() {
	int status;
	pid_t pid;
	pid_t parent_pid;
	sigset_t block_quit;

	sigemptyset(&block_quit);
	sigaddset(&block_quit, SIGQUIT);
	sigprocmask(SIG_BLOCK, &block_quit, NULL);
	parent_pid = getpid();
	kill(-parent_pid, SIGQUIT);
	while((pid = waitpid(-1, &status, 0)) > 0) {
		printf("Background process %d terminated.\n", pid);
	}
	printf("Shutting down...\n");
	exit(0);
}
