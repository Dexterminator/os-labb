#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#define READ_END 0
#define WRITE_END 1
#define STDIN_INT 0
#define STDOUT_INT 1

void test_pipe(char* command) {
	int pid1, pid2, pid3;
	int pipe1[2];
	int pipe2[2];
	int status;

	pipe(pipe1);
	pipe(pipe2);
	pid1 = fork();
	if (pid1 == 0) {
		redirect_standard_out(pipe1[WRITE_END]);
		close_pipe(pipe1);
		close_pipe(pipe2);
		execlp("printenv", "printenv", NULL);
	}

	pid2 = fork();
	if (pid2 == 0) {
		redirect_standard_in(pipe1[READ_END]);
		redirect_standard_out(pipe2[WRITE_END]);
		close_pipe(pipe1);
		close_pipe(pipe2);
		execlp("sort", "sort", NULL);
	}

	pid3 = fork();
	if (pid3 == 0) {
		redirect_standard_in(pipe2[READ_END]);
		close_pipe(pipe1);
		close_pipe(pipe2);
		execlp("less", "less", NULL);
	}

	close_pipe(pipe1);
	close_pipe(pipe2);
	waitpid(pid3, &status, 0);
	printf("%s\n", "piping done");
}

void redirect_standard_in(int pipe_read_end) {
	dup2(pipe_read_end, STDIN_INT);
}

void redirect_standard_out(int pipe_write_end) {
	dup2(pipe_write_end, STDOUT_INT);
}

void close_pipe(int pipe[2]) {
	close(pipe[READ_END]);
	close(pipe[WRITE_END]);
}