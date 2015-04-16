#include <stdio.h>
#include <string.h>
#include "cd.h"
void get_command();
void handle_command();
int starts_with(char*, char*);
void substring_to_end(char* result, char* string, size_t start);
void substring(char* result, char* string, size_t start, size_t end);

int main()
{
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
	char path[80];
	if (starts_with(command, "cd")) {
		substring_to_end(path, command, 3);
		change_working_directory(path);
	} else {
		printf("Unknown command '%s'\n", command);
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