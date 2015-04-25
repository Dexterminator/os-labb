#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#define READ_END 0
#define WRITE_END 1

void test_pipe(char* command) {
	int pid;
	int fd[2];
	pipe(fd);
	pid = fork();
	printf("%d\n", pid);

	if (pid == 0) {
		dup2(fd[WRITE_END], fileno(stdout));
		close(fd[READ_END]);
		execlp("printenv", "printenv", NULL);
	} else {
		dup2(fd[READ_END], fileno(stdin));
		close(fd[WRITE_END]);
		execlp("sort", "sort", NULL);
	}
}