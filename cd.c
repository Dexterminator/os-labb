#include <stdio.h>
#include <unistd.h>
#include <errno.h>
void print_working_directory();
void handle_error();

void print_working_directory() {
	char working_directory[100];
	char* working_directory_ptr;

	working_directory_ptr = getcwd(working_directory, 2);
	if (working_directory_ptr == NULL) {
		handle_error();
	} else {
		printf("%s\n", working_directory);
	}
}

void handle_error() {
	switch(errno) {
		case EINVAL: printf("Buffer was zero, what the hell is wrong with you?\n");
		break;
		case ERANGE: printf("Size argument is less than working directory length.\n");
		break;
		default: printf("I have never seen this error in my whole life.\n");
	}
}