#include <stdio.h>
int main()
{
	char str[80];
	int successful_scan;

	while(1) {
		printf("Enter a value: ");
		successful_scan = scanf("%s", str);
		if (!successful_scan)
			printf("Did not scan line succesfully");

		printf("\nYou entered: %s\n", str);
	}
	return 0;
}