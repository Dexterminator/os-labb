#include <stdio.h>
#include <errno.h>
#include <string.h>
void print_error();
void print_error() {
	printf("Error: %s\n", strerror(errno));
}