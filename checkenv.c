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
int pipe_printenv();
int pipe_grep();
int pipe_sort();
int pipe_pager();
int redirect_standard_in(int pipe_read_end);
int redirect_standard_out(int pipe_write_end);
int exec_printenv();
int exec_sort();
int exec_pager();
int exec_grep(char* const* arguments);
int setup_pipes(int argc);
void set_pipe_identifiers(int argc);

/* The checkEnv command. Sets up pipes and their identifiers
 * between printenv, (optionally) grep, sort, and pager, and
 * executes the pipeline.
 */
void checkenv(char* const* arguments, int argc) {
	pager = getenv("PAGER");
	if (setup_pipes(argc) == -1) {
		return;
	}
	set_pipe_identifiers(argc);

	if(pipe_printenv() == -1) {
		return;
	}
	if (argc > 1) {
		if(pipe_grep(arguments) == -1) {
			return;
		}
	}
	if(pipe_sort() == -1) {
		printf("sort failed.\n");
		return;
	}
	if(pipe_pager() == -1) {
		return;
	}
}

/* Spawns a child process, calls exec_printenv in
 * this process, closes the proper file descriptor
 * and waits for the child process to terminate.
 */
int pipe_printenv() {
	pid_t pid;
	pid = fork();
	if (pid == -1) {
		perror("fork");
		return -1;
	}

	if (pid == 0) {
		if(exec_printenv() == -1) {
			_exit(1);
		}
	}

	if(close(post_printenv[WRITE_END]) == -1) {
		perror("close");
		return -1;
	}
	if(wait(&status) == -1) {
		perror("wait");
		return -1;
	}
	return 1;
}

/* Spawns a child process, calls exec_grep in
 * this process, closes the proper file descriptors
 * and waits for the child process to terminate.
 */
int pipe_grep(char* const* arguments) {
	pid_t pid;

	pid = fork();
	if (pid == -1) {
		perror("fork");
		return -1;
	}
	if (pid == 0) {
		if(exec_grep(arguments) == -1) {
			_exit(1);
		}
	}

	if (close(pre_grep[READ_END]) == -1) {
		perror("close");
		return -1;
	}

	if(close(post_grep[WRITE_END]) == -1) {
		perror("close");
		return -1;
	}

	if(wait(&status) == -1) {
		perror("close");
		return -1;
	}
	return 1;
}

/* Spawns a child process, calls exec_sort in
 * this process, closes the proper file descriptors
 * and waits for the child process to terminate.
 */
int pipe_sort() {
	pid_t pid;

	pid = fork();
	if (pid == -1) {
		perror("fork");
		return -1;
	}

	if (pid == 0) {
		if(exec_sort() == -1) {
			_exit(1);
		}
	}
	if(close(pre_sort[READ_END]) == -1) {
		perror("close");
		return -1;
	}
	if(close(post_sort[WRITE_END]) == -1) {
		perror("close");
		return -1;
	}
	if(wait(&status) == -1) {
		perror("wait");
		return -1;
	}
	return 1;
}

/* Spawns a child process, calls exec_pager in
 * this process, closes the proper file descriptor
 * and waits for the child process to terminate.
 */
int pipe_pager() {
	pid_t pid;

	pid = fork();
	if(pid == -1) {
		perror("fork");
		return -1;
	}
	if (pid == 0) {
		if(exec_pager() == -1) {
			_exit(1);
		}
	}
	if(close(pre_pager[READ_END]) == -1) {
		perror("close");
		return -1;
	}
	if(wait(&status) == -1) {
		perror("wait");
		return -1;
	}
	return 1;
}

/* Sets up the pipes (by calling pipe() to setup file descriptors representing the ends of the pipes)
 * according to the argument count. If the user has supplied any arguments, an extra pipe is needed
 * in order to execute grep.
 */
int setup_pipes(int argc) {
	if (pipe(pipe1) == -1) {
		perror("pipe");
		return -1;
	}

	if (argc > 1) {
		if (pipe(pipe2) == -1) {
			perror("pipe");
			return -1;
		}
	}

	if (pipe(pipe3) == -1) {
		perror("pipe");
		return -1;
	}
	return 1;
}

/* Set different aliases for pipes based on the number of arguments.
 * The aliases represent at what stage in the pipeline the pipe is
 * located.
 */
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

/* Redirects the current processes standard output to the proper pipe.
 * Then closes the unused end of the pipe, and executes printenv.
 */
int exec_printenv() {
	if(redirect_standard_out(post_printenv[WRITE_END]) == -1) {
		perror("redirect_standard_out");
		return -1;
	}
	if(close(post_printenv[READ_END]) == -1) {
		perror("close");
		return -1;
	}
	if(execlp("printenv", "printenv", NULL) == -1) {
		perror("execlp");
		return -1;
	}
	return 1;
}

/* Redirects the current processes standard input and output to the proper pipes.
 * Then closes the unused end of the pipe, and executes sort.
 */
int exec_sort() {
	if(redirect_standard_in(pre_sort[READ_END]) == -1) {
		perror("redirect_standard_in");
		return -1;
	}
	if(redirect_standard_out(post_sort[WRITE_END]) == -1) {
		perror("redirect_standard_out");
		return -1;
	}
	if(close(post_sort[READ_END]) == -1) {
		perror("close");
		return -1;
	}
	if(execlp("sort", "sort", NULL) == -1) {
		perror("execlp");
		return -1;
	}
	return 1;
}

/* Redirects the current processes standard input to the proper pipe.
 * Then closes the unused end of the pipe, and executes:
 * - whatever the PAGER environment variable is set to if it is set
 * - less, if available
 * - more, otherwise
 */
int exec_pager() {
	if(redirect_standard_in(pre_pager[READ_END]) == -1) {
		perror("redirect_standard_in");
		return -1;
	}
	if (pager != NULL) {
		if(execlp(pager, pager, NULL) == -1) {
			perror("execlp");
			return -1;
		}
	} else {
		if (execlp("less", "less", NULL) == -1) {
			if(execlp("more", "more", NULL) == -1) {
				perror("execlp");
				return -1;
			}
		}
	}
	return 1;
}

/* Redirects the current processes standard input and output to the proper pipes.
 * Then closes the unused end of the pipe, and executes grep.
 */
int exec_grep(char* const* arguments) {
	if(redirect_standard_in(pre_grep[READ_END]) == -1) {
		perror("redirect_standard_in");
		return -1;
	}
	if(redirect_standard_out(post_grep[WRITE_END]) == -1) {
		perror("redirect_standard_out");
		return -1;
	}
	if(close(post_grep[READ_END]) == -1) {
		perror("close");
		return -1;
	}
	if(execvp("grep", arguments) == -1) {
		perror("grep");
		return -1;
	}
	return 1;
}

/* Uses dup2 to redirect standard input.
 */
int redirect_standard_in(int pipe_read_end) {
	return dup2(pipe_read_end, STDIN_INT);
}

/* Uses dup2 to redirect standard output.
 */
int redirect_standard_out(int pipe_write_end) {
	return dup2(pipe_write_end, STDOUT_INT);
}
