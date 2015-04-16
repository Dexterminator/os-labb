#include <stdio.h>
#include <unistd.h>
#include <errno.h>
void change_working_directory();
void print_working_directory();
void handle_chdir_error();
void handle_getcwd_error();

void change_working_directory(char* path) {
	int ok_code;
	ok_code = chdir(path);
	if (ok_code == -1) {
		handle_chdir_error();
	}
}

void print_working_directory() {
	char working_directory[100];
	char* working_directory_ptr;

	working_directory_ptr = getcwd(working_directory, 100);
	if (working_directory_ptr == NULL) {
		handle_getcwd_error();
		return;
	}
	printf("%s", working_directory);
}

void handle_chdir_error() {
	switch(errno) {
		case EACCES: printf("EACCES\n");
		break;
		default: printf("Unknown error code.\n");
	}
}

void handle_getcwd_error() {
	switch(errno) {
		case EINVAL: printf("Buffer was zero, what the hell is wrong with you?\n");
		break;
		case ERANGE: printf("Size argument is less than working directory length.\n");
		break;
		default: printf("I have never seen this error in my whole life.\n");
	}
}