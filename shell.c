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
void setup_termination_handler();
char* home;
pid_t parent_pid;
struct sigaction detection_sa;

int main() {
	parent_pid = getpid();
	home = getenv("HOME");
	change_working_directory(NULL, 1, home);
	setup_termination_handler();

	if (SIGHANDLER) {
		detection_sa.sa_handler = &detection_sighandler;
		detection_sa.sa_flags = SA_RESTART;
		sigemptyset(&detection_sa.sa_mask);

		printf("Using signal handler.\n");
		if (sigaction(SIGCHLD, &detection_sa, 0) == -1) {
		  perror(0);
		  exit(1);
		}
	} else {
		printf("Using polling.\n");
	}
	get_command();
	return 0;
}

void setup_termination_handler() {
	struct sigaction termination_sa;

	termination_sa.sa_handler = &termination_sighandler;
	termination_sa.sa_flags = 0;
	sigemptyset(&termination_sa.sa_mask);
	sigaction(SIGQUIT, &termination_sa, 0);

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

void detection_sighandler(int signum) {
	pid_t pid;
	int status;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		fprintf(stderr, "Background process %d terminated.\n", pid);
	}
}

void termination_sighandler(int signum) {
	if (parent_pid != getpid()) {
		_exit(0);
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
		exit_shell(parent_pid);
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

	arguments[arg_number - 1] = NULL;
	pid = fork();
	if (pid == 0) {
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
