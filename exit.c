#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include "helper.h"
void exit_shell();

/* The exit command. Sets up a process signal mask for the main process
 * that block SIGQUIT. It then sends SIGQUIT using kill(2) to signal
 * all child processes that they should terminate (which is done through a signal handler).
 * After this, it waits until they have all terminated using waitpid, after which the main
 * process exits.
 */
void exit_shell() {
	int status;
	pid_t pid;
	pid_t parent_pid;
	sigset_t block_quit;

	sigemptyset(&block_quit);
	sigaddset(&block_quit, SIGQUIT);
	if (sigprocmask(SIG_BLOCK, &block_quit, NULL) == -1){
		print_error();
		exit(1);
	}
	parent_pid = getpid();
	if (kill(-parent_pid, SIGQUIT) == -1) {
		print_error();
		exit(1);
	}
	while((pid = waitpid(-1, &status, 0)) > 0) {
		printf("Background process %d terminated.\n", pid);
	}
	printf("Shutting down...\n");
	exit(0);
}
