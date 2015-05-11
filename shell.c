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
void detection_sighandler(int signum);
void termination_sighandler(int signum);
void interruption_sighandler(int signum);
void setup_termination_handler();
void setup_interruption_handler();
void setup_detection_handler();
char* home;

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

void setup_detection_handler() {
	struct sigaction detection_sa;
	detection_sa.sa_handler = &detection_sighandler;
	detection_sa.sa_flags = SA_RESTART;
	sigemptyset(&detection_sa.sa_mask);

	printf("Using signal handler.\n");
	if (sigaction(SIGCHLD, &detection_sa, 0) == -1) {
		print_error();
	  exit(1);
	}
}

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

void interruption_sighandler(int signum) {
	tcflush(fileno(stdin), TCIFLUSH);
	fprintf(stdout, "\n");
	print_working_directory();
	printf(" > ");
	fflush(stdout);
}

void detection_sighandler(int signum) {
	pid_t pid;
	int status;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		fprintf(stderr, "Background process %d terminated.\n", pid);
	}
}

void termination_sighandler(int signum) {
	_exit(0);
}

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
		command[strlen(command) - 1] = '\0';
		if (successful_read == NULL) {
			printf("Did not scan line succesfully");
			print_error();
		}
		handle_command(command);
	}
}

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

void handle_exec(char** arguments, int arg_number, char* command) {
	int is_background;

	is_background = string_equals(arguments[arg_number - 1], "&");
	if (is_background) {
		exec_background(arguments, arg_number, command);
	} else {
		exec_foreground(arguments, arg_number, command);
	}
}

void find_terminated_with_polling() {
	pid_t pid;
	int poll_status;

	while((pid = waitpid(-1, &poll_status, WNOHANG)) > 0) {
		printf("Background process %d terminated.\n", pid);
	}
}

void exec_background(char** arguments, int arg_number, char* command) {
	pid_t pid;
	sigset_t block_int;

	arguments[arg_number - 1] = NULL;
	pid = fork();
	if (pid == 0) {
		sigemptyset(&block_int);
		sigaddset(&block_int, SIGINT);
		sigprocmask(SIG_BLOCK, &block_int, NULL);
		execvp(command, arguments);
	}
	printf("Spawned background process pid: %d\n", pid);
}

void exec_foreground(char** arguments, int arg_number, char* command) {
	pid_t pid;
	struct timeval start, end;
	int exec_ok;
	int status;

	pid = fork();
	if (pid == 0) {
		exec_ok = execvp(command, arguments);
		if (exec_ok == -1) {
			print_error();
			_exit(0);
		}
	}
	printf("Spawned foreground process pid: %d\n", pid);
	gettimeofday(&start, NULL);
	waitpid(pid, &status, 0);
	gettimeofday(&end, NULL);
	printf("Foreground process %d terminated\n", pid);
	printf("Process %d: Time elapsed: %f\n", pid, time_difference(&start, &end));
}
