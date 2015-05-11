#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
void print_error();
/* Prints "Error: [reason]"
 */
void print_error() {
	perror("Error");
}

/* Calculates the time difference in milliseconds between to given timeval structs.
 */
double time_difference(const struct timeval* start, const struct timeval* end) {
	double start_millis;
	double end_millis;

	start_millis = start->tv_sec * 1000 + (double) start->tv_usec / 1000;
	end_millis = end->tv_sec * 1000 + (double) end->tv_usec / 1000;
	return end_millis - start_millis;
}

/* Returns 1 if the provided strings are exactly equal.
*/
int string_equals(char* string1, char* string2) {
	return strcmp(string1, string2) == 0;
}
