#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
void print_error();
void print_error() {
	printf("Error: %s\n", strerror(errno));
}

double time_difference(const struct timeval* start, const struct timeval* end) {
	double start_millis;
	double end_millis;
	start_millis = start->tv_sec * 1000 + (double) start->tv_usec / 1000;
	end_millis = end->tv_sec * 1000 + (double) end->tv_usec / 1000;
	return end_millis - start_millis;
}

int string_equals(char* string1, char* string2) {
	return strcmp(string1, string2) == 0;
}
