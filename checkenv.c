#include <stdio.h>
/*void test_pipe(char* command) {
	FILE* pipe1;
	FILE* pipe2;
	FILE* pipe3;
	char command_out[1024];
	pipe1 = popen("printenv", "r");
	printf("%s\n", "wat0");
	pipe2 = popen("sort", "w");
	printf("%s\n", "wat");
	while (fgets(command_out, 1024, pipe1) != NULL)
		fputs(command_out, pipe2);
	printf("%s\n", "wat2");
	pclose(pipe1);
	pclose(pipe2);
	pipe3 = popen("sort" "r");
}*/