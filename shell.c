#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/time.h>
#include "cd.h"
#include "checkenv.h"
#include "helper.h"
void get_command();
void handle_command();
int starts_with(char*, char*);
int string_equals(char* string1, char* string2);
void substring_to_end(char* result, char* string, size_t start);
void substring(char* result, char* string, size_t start, size_t end);
void handle_cd(char* command);
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
	int poll_status;
	pid_t child_pid;

	while(1) {

		/* Child termination detection by polling */
		while((child_pid = waitpid(-1, &poll_status, WNOHANG)) > 0) {
			printf("Process %d terminated.\n", child_pid);
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

void handle_command(char* input) {
	char* command;
	char* argument;
	char* arguments[10];
	int arg_number;
	pid_t pid;
	int status;
	struct timeval start, end;
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
		int is_background = string_equals(arguments[arg_number - 1], "&");
		if (is_background) {
			arguments[arg_number - 1] = NULL;
			printf("%s\n", "background prcs");
			pid = fork();
			if (pid == 0) {
				execvp(command, arguments);
			}
			printf("Spawned background process pid: %d\n", pid);
		} else {
			/* Foreground process */
			pid = fork();
			if (pid == 0) {
				execvp(command, arguments);
			}
			printf("Spawned foreground process pid: %d\n", pid);
			gettimeofday(&start, NULL);
			waitpid(pid, &status, 0);
			gettimeofday(&end, NULL);
			printf("Foreground process %d terminated\n", pid);
			printf("Time elapsed: %f\n", time_difference(&start, &end));
		}
	}
}

int string_equals(char* string1, char* string2) {
	return strcmp(string1, string2) == 0;
}

int starts_with(char* base, char* str) {
	return (strstr(base, str) - base) == 0;
}

void substring_to_end(char* result, char* string, size_t start) {
	substring(result, string, start, strlen(string));
}

void substring(char* result, char* string, size_t start, size_t end) {
	if (end > strlen(string) || start > strlen(string))
		return;
	strncpy(result, &string[start], end - start);
	result[end - start] = '\0';
}
