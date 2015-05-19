#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <termios.h>
#include "cd.h"
#include "checkenv.h"
#include "helper.h"
#include "exit.h"
#ifdef SIGDET
#define SIGHANDLER SIGDET
#endif
#ifndef SIGDET
#define SIGHANDLER 0
#endif
void get_command();
void handle_command();
void handle_cd(char* command);
void find_terminated_with_polling();
void handle_exec(char** arguments, int arg_number, char* command);
void exec_background(char** arguments, int arg_number, char* command);
void exec_foreground(char**, int arg_number, char* command);
void termination_sighandler(int signum);
void interruption_sighandler(int signum);
void setup_termination_handler();
void setup_interruption_handler();
void setup_detection_handler();
char* home;

/* Entry point of program. Sets up signal handlers for interruptions and terminations.
 * if SIGDET=1 is used at compile time, also set up a signal handler for detecting terminated
 * background processes. Then initializes the input execute loop.
 */
int main() {
	home = getenv("HOME");
	change_working_directory(NULL, 1, home);
	setup_termination_handler();
	setup_interruption_handler();

	if (SIGHANDLER) {
		setup_detection_handler();
	} else {
		printf("Using polling.\n");
	}
	get_command();
	return 0;
}

/* Set up a signal handler that detects terminated processes (through SIGCHLD).
 * SA_RESTART flag is set in order to automatically restart system calls that
 * are interrupted.
 */
void setup_detection_handler() {
	struct sigaction detection_sa;
	detection_sa.sa_handler = &find_terminated_with_polling;
	detection_sa.sa_flags = SA_RESTART;
	sigemptyset(&detection_sa.sa_mask);

	printf("Using signal handler.\n");
	if (sigaction(SIGCHLD, &detection_sa, 0) == -1) {
		print_error();
	  exit(1);
	}
}

/* Set up a signal handler that handles interruption (SIGINT) signals.
 * SA_RESTART flag is set in order to automatically restart system calls that
 * are interrupted.
 */
void setup_interruption_handler() {
	struct sigaction interruption_sa;

	interruption_sa.sa_handler = &interruption_sighandler;
	interruption_sa.sa_flags = SA_RESTART;
	sigemptyset(&interruption_sa.sa_mask);
	if(sigaction(SIGINT, &interruption_sa, 0) == -1) {
		print_error();
		exit(1);
	}
}

/* Set up a signal handler that handles SIGQUIT signals.
 */
void setup_termination_handler() {
	struct sigaction termination_sa;

	termination_sa.sa_handler = &termination_sighandler;
	termination_sa.sa_flags = 0;
	sigemptyset(&termination_sa.sa_mask);
	if(sigaction(SIGQUIT, &termination_sa, 0) == -1) {
		print_error();
		exit(1);
	}
}

/* On interruption, flush standard in stream and print prompt again.
 * By intercepting this signal, we avoid that the main process is
 * interrupted by ctr+c.
 */
void interruption_sighandler(int signum) {
	if(tcflush(fileno(stdin), TCIFLUSH) == -1) {
		perror("tcflush");
	}
	fprintf(stdout, "\n");
	print_working_directory();
	printf(" > ");
	if(fflush(stdout) == EOF) {
		perror("fflush");
	}
}

/* Simply terminates current process. Used by the exit command to
 * terminate all child processes.
 */
void termination_sighandler(int signum) {
	_exit(0);
}

/* The main input loop. Takes input from the user and forwards it
 * to the handle_command function. If SIGDET is not set to 1 at
 * compile time, it prints all background processes that have
 * terminated since the last prompt.
 */
void get_command() {
	char command[80];
	char* successful_read;

	while(1) {
		if (!SIGHANDLER) {
			find_terminated_with_polling();
		}
		print_working_directory();
		printf(" > ");
		successful_read = fgets(command, sizeof(command), stdin);
		if (successful_read == NULL) {
			printf("Did not scan line succesfully");
			print_error();
		}
		command[strlen(command) - 1] = '\0'; /* Add null character to properly end command.*/
		handle_command(command);
	}
}

/* Tokenizes input and separates it into the command itself and its argument list.
 * After this, check if the command matches any of the built in commands, and
 * execute them if they do. Otherwise, try to execute the command using handle_exec.
 */
void handle_command(char* input) {
	char* command;
	char* argument;
	char* arguments[10];
	int arg_number;

	command = strtok(input, " ");
	if (command == NULL) {
		return;
	}

	arguments[0] = command;
	arg_number = 1;
	argument = strtok(NULL, " ");

	while(argument != NULL) {
		arguments[arg_number] = argument;
		arg_number++;
		argument = strtok(NULL, " ");
	}
	arguments[arg_number] = NULL;

	if (string_equals(command, "cd")) {
		change_working_directory(arguments, arg_number, home);
	} else if (string_equals(command, "checkEnv")) {
		checkenv(arguments, arg_number);
	} else if (string_equals(command, "exit")) {
		exit_shell();
	} else {
		handle_exec(arguments, arg_number, command);
	}
}

/* Checks the last character of the argument list in order to decide
 * if the command should be executed by a foreground or a background
 * process, and executes the corresponding function.
 */
void handle_exec(char** arguments, int arg_number, char* command) {
	int is_background;

	is_background = string_equals(arguments[arg_number - 1], "&");
	if (is_background) {
		exec_background(arguments, arg_number, command);
	} else {
		exec_foreground(arguments, arg_number, command);
	}
}

/* Detects and reaps zombie processes by waitpid without hanging,
 * meaning that we will not wait infinitely if there are no terminated
 * children. When a process is detected, it is printed to the command line.
 */
void find_terminated_with_polling() {
	pid_t pid;
	int poll_status;

	while((pid = waitpid(-1, &poll_status, WNOHANG)) > 0) {
		printf("Background process %d terminated.\n", pid);
	}
}

/* Spawn a child process (which the main process does not wait for),
 * and use execvp to execute the given command. Also set a process signal
 * mask that makes sure that SIGINT does not reach the process (as we do
 * not want ctrl+c to interrup background processes).
 */
void exec_background(char** arguments, int arg_number, char* command) {
	pid_t pid;
	sigset_t block_int;

	arguments[arg_number - 1] = NULL;
	pid = fork();
	if (pid == 0) {
		sigemptyset(&block_int);
		sigaddset(&block_int, SIGINT);
		if(sigprocmask(SIG_BLOCK, &block_int, NULL) == -1) {
			perror("sigprocmask");
			_exit(1);
		}
		if(execvp(command, arguments) == -1) {
			perror("execvp");
			_exit(1);
		}
	}
	printf("Spawned background process pid: %d\n", pid);
}

/* Spawns a child process, uses execvp to execute the given command,
 * and lets the main process wait for the child to finish. Uses gettimeofday
 * before and after execution in order to calculate execution time and print it
 * to the command line.
 */
void exec_foreground(char** arguments, int arg_number, char* command) {
	pid_t pid;
	struct timeval start, end;
	int status;
	int time_ok;

	pid = fork();
	if (pid == 0) {
		if(execvp(command, arguments) == -1) {
			print_error();
			_exit(0);
		}
	}
	printf("Spawned foreground process pid: %d\n", pid);
	time_ok = gettimeofday(&start, NULL);
	if(time_ok == -1) {
		perror("gettimeofday");
	}
	waitpid(pid, &status, 0);
	printf("Foreground process %d terminated\n", pid);
	if (time_ok != -1) {
		if(gettimeofday(&end, NULL) == -1) {
			perror("gettimeofday");
		} else {
			printf("Process %d: Time elapsed: %f\n", pid, time_difference(&start, &end));
		}
	}
}
