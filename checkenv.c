#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#define READ_END 0
#define WRITE_END 1
#define STDIN_INT 0
#define STDOUT_INT 1
pid_t pid1, pid2, pid3;
int pipe1[2];
int pipe2[2];
int pipe3[2];
int status;
int less_error;
int pipe_error;
char* pager;
void checkenv();
void redirect_standard_in(int pipe_read_end);
void redirect_standard_out(int pipe_write_end);
void close_pipe(int pipe[2]);
void close_pipes(int* pipe1, int* pipe2);
void exec_printenv();
void exec_sort();
void exec_pager();

void checkenv() {
	pager = getenv("PAGER");

	if (pipe(pipe1) == -1) {
		/* code */
	}

	if (pipe(pipe2) == -1) {
		/* code */
	}
	pid1 = fork();
	if (pid1 == 0) {
		exec_printenv();
	}

	pid2 = fork();
	if (pid2 == 0) {
		exec_sort();
	}

	pid3 = fork();
	if (pid3 == 0) {
		exec_pager();
	}

	close_pipes(pipe1, pipe2);
	waitpid(pid3, &status, 0);
}

void exec_printenv() {
	redirect_standard_out(pipe1[WRITE_END]);
	close_pipes(pipe1, pipe2);
	execlp("printenv", "printenv", NULL);
}

void exec_sort() {
	redirect_standard_in(pipe1[READ_END]);
	redirect_standard_out(pipe2[WRITE_END]);
	close_pipes(pipe1, pipe2);
	execlp("sort", "sort", NULL);
}

void exec_pager() {
	redirect_standard_in(pipe2[READ_END]);
	close_pipes(pipe1, pipe2);
	if (pager != NULL) {
		execlp(pager, pager, NULL);
	} else {
		less_error = execlp("less", "less", NULL);
		if (less_error == -1) {
			execlp("more", "more", NULL);
		}
	}
}

void redirect_standard_in(int pipe_read_end) {
	dup2(pipe_read_end, STDIN_INT);
}

void redirect_standard_out(int pipe_write_end) {
	dup2(pipe_write_end, STDOUT_INT);
}

void close_pipes(int* pipe1, int* pipe2) {
	close_pipe(pipe1);
	close_pipe(pipe2);
}

void close_pipe(int* pipe) {
	close(pipe[READ_END]);
	close(pipe[WRITE_END]);
}
