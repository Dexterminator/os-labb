void checkenv();
void redirect_standard_in(int pipe_read_end);
void redirect_standard_out(int pipe_write_end);
void close_pipe(int pipe[2]);
void close_pipes(int* pipe1, int* pipe2);
void exec_printenv();
void exec_sort();
void exec_pager();
