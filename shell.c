#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cd.h"
#include "checkenv.h"
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
	change_working_directory(NULL, 0, home);
	get_command();
	return 0;
}

void get_command() {
	char command[80];
	char* successful_read;

	while(1) {
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
	command = strtok(input, " ");
	arg_number = 0;
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
		printf("Unknown command: '%s'\n", command);
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
