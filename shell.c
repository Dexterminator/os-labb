#include <stdio.h>
#include "cd.h"
void get_command();
int main()
{
	get_command();
	return 0;
}

void get_command() {
	char command[80];
	int successful_scan;

	while(1) {
		printf("> ");
		successful_scan = scanf("%s", command);
		if (!successful_scan)
			printf("Did not scan line succesfully");

		printf("You entered: %s\n", command);
		print_working_directory();
	}
}