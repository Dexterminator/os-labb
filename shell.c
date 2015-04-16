#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cd.h"
void get_command();
void handle_command();
int starts_with(char*, char*);
void substring_to_end(char* result, char* string, size_t start);
void substring(char* result, char* string, size_t start, size_t end);
void handle_cd(char* command);
char* home;

int main()
{
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
		if (successful_read == NULL)
			printf("Did not scan line succesfully");
		handle_command(command);
	}
}

void handle_command(char* command) {
	if (starts_with(command, "cd")) {
		handle_cd(command);
	} else {
		printf("Unknown command '%s'\n", command);
	}
}

void handle_cd(char* command) {
	char path[80];
	if (strcmp(command, "cd") == 0) {
		change_working_directory(home);
	} else {
		substring_to_end(path, command, 3);
		change_working_directory(path);
	}
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