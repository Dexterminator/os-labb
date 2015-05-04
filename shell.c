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
void sighandler(int signum);
char* home;

int main() {
	home = getenv("HOME");
	change_working_directory(NULL, 1, home);
	get_command();
	return 0;
}

void get_command() {
	char command[80];
	char* successful_read;
	signal(SIGINT, sighandler);

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
		}
		handle_command(command);
	}
}

void sighandler(int signum) {
	printf("Caught signal: %d\n", signum);
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
	printf("%s\n", "background prcs");
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
	printf("Time elapsed: %f\n", time_difference(&start, &end));
}
