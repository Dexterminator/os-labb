#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "helper.h"
void change_working_directory(char* const* arguments, int argc, char* home);
void print_working_directory();
void handle_chdir_error();
void handle_getcwd_error();

void change_working_directory(char* const* arguments, int argc, char* home) {
	int ok;
	if(argc == 1) {
		ok = chdir(home);
	} else if (argc > 1) {
		ok = chdir(arguments[1]);
	} else {
		printf("Something went wrong.\n");
		return;
	}

	if (ok == -1) {
		print_error();
	}
}

void print_working_directory() {
	char working_directory[100];
	char* working_directory_ptr;

	working_directory_ptr = getcwd(working_directory, 100);
	if (working_directory_ptr == NULL) {
		print_error();
		return;
	}
	printf("%s", working_directory);
}
