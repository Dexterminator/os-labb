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
	if(argc > 1) {
		printf("%s\n", "Du är fan keff.");
		return;
	} else if (argc == 0) {
		ok = chdir(home);
	} else {
		ok = chdir(arguments[0]);
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
