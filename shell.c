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
	change_working_directory(home);
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

void handle_command(char* command) {
	char* token;
	char* checkenv_arguments[10];
	int arg_number;
	token = strtok(command, " ");
	if (string_equals(token, "cd")) {
		token = strtok(NULL, " ");
		handle_cd(token);
	} else if (string_equals(token, "checkEnv")) {
		token = strtok(NULL, " ");
		if (token != NULL) {
			checkenv_arguments[0] = "grep";
			checkenv_arguments[1] = token;
			arg_number = 2;
			while(token != NULL) {
				token = strtok(NULL, " ");
				checkenv_arguments[arg_number] = token;
				arg_number++;
			}
			checkenv_arguments[arg_number] = NULL;
			checkenv(checkenv_arguments);
		} else {
			checkenv(NULL);
		}
	} else {
		printf("Unknown command: '%s'\n", token);
	}
}

void handle_cd(char* path) {
	path = path == NULL ? home : path;
	change_working_directory(path);
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