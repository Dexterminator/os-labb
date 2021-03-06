#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "helper.h"
void change_working_directory(char* const* arguments, int argc, char* home);
void print_working_directory();
void handle_chdir_error();
void handle_getcwd_error();

/* Calls chdir() with the provided argument. If no argument is provided,
 * the home directory is used instead.
 */
void change_working_directory(char* const* arguments, int argc, char* home) {
	int err;

	if(argc == 1) {
		err = chdir(home);
	} else if (argc > 1) {
		err = chdir(arguments[1]);
	} else {
		printf("Something went wrong.\n");
		return;
	}

	if (err == -1) {
		print_error();
	}
}

/* Prints the current working directory.
 */
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
