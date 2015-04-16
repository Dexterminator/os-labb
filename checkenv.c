#include <stdio.h>
void test_pipe(char* command) {
	FILE* pipe;
	char command_out[1024];
	pipe = popen("printenv", "r");
	if (pipe == NULL) {
		/* Handle error */
	}
	while (fgets(command_out, 1024, pipe) != NULL)
    printf("%s", command_out);
}