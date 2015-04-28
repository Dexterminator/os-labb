void checkenv(char* const* arguments);
void redirect_standard_in(int pipe_read_end);
void redirect_standard_out(int pipe_write_end);
void close_pipe(int pipe[2]);
void close_pipes(int* pipe1, int* pipe2);
void exec_printenv(int* pipe1, int* pipe2);
void exec_sort(int pipe1[2], int pipe2[2]);
void exec_pager(int pipe1[2], int pipe2[2]);
void exec_grep(char* const* arguments);
