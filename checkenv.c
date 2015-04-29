#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "helper.h"
#define READ_END 0
#define WRITE_END 1
#define STDIN_INT 0
#define STDOUT_INT 1
pid_t pid;
int pipe1[2];
int pipe2[2];
int pipe3[2];
int* post_printenv;
int* pre_sort;
int* pre_grep;
int* post_grep;
int* post_sort;
int* pre_pager;
int status;
int less_error;
int pipe_error;
char* pager;
void checkenv(char* const* arguments, int argc);
void redirect_standard_in(int pipe_read_end);
void redirect_standard_out(int pipe_write_end);
void close_pipe(int* pipe);
void close_pipes(int* pipe1, int* pipe2);
void exec_printenv();
void exec_sort();
void exec_pager();
void exec_grep(char* const* arguments);
int setup_pipes(int argc);
void set_pipe_identifiers(int argc);

void checkenv(char* const* arguments, int argc) {
	pager = getenv("PAGER");
	if (setup_pipes(argc) == -1) {
		return;
	}
	set_pipe_identifiers(argc);

	pid = fork();
	if (pid == 0) {
		exec_printenv();
	}
	close(post_printenv[WRITE_END]);
	wait(&status);

	if (argc > 1) {
		pid = fork();
		if (pid == 0) {
			exec_grep(arguments);
		}
		close(pre_grep[READ_END]);
		close(post_grep[WRITE_END]);
		wait(&status);
	}

	pid = fork();
	if (pid == 0) {
		exec_sort();
	}
	close(pre_sort[READ_END]);
	close(post_sort[WRITE_END]);
	wait(&status);

	pid = fork();
	if (pid == 0) {
		exec_pager();
	}
	close(pre_pager[READ_END]);
	wait(&status);
}

int setup_pipes(int argc) {
	if (pipe(pipe1) == -1) {
		print_error();
		return -1;
	}

	if (argc > 1) {
		if (pipe(pipe2) == -1) {
			print_error();
			return -1;
		}
	}

	if (pipe(pipe3) == -1) {
		print_error();
		return -1;
	}
	return 1;
}

void set_pipe_identifiers(int argc) {
	post_printenv = pipe1;
	pre_pager = pipe3;
	post_sort = pipe3;
	if (argc > 1) {
		pre_grep = pipe1;
		post_grep = pipe2;
		pre_sort = pipe2;
	} else {
		pre_sort = pipe1;
	}
}

void exec_printenv() {
	redirect_standard_out(post_printenv[WRITE_END]);
	close(post_printenv[READ_END]);
	execlp("printenv", "printenv", NULL);
}

void exec_sort() {
	redirect_standard_in(pre_sort[READ_END]);
	redirect_standard_out(post_sort[WRITE_END]);
	close(pre_sort[WRITE_END]);
	close(post_sort[READ_END]);
	execlp("sort", "sort", NULL);
}

void exec_pager() {
	redirect_standard_in(pre_pager[READ_END]);
	close(pre_pager[WRITE_END]);
	if (pager != NULL) {
		execlp(pager, pager, NULL);
	} else {
		if (execlp("less", "less", NULL) == -1) {
			execlp("more", "more", NULL);
		}
	}
}

void exec_grep(char* const* arguments) {
	redirect_standard_in(pre_grep[READ_END]);
	redirect_standard_out(post_grep[WRITE_END]);
	close(pre_grep[WRITE_END]);
	close(post_grep[READ_END]);
	execvp("grep", arguments);
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
