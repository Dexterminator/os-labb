void checkenv();
void redirect_standard_in(int pipe_read_end);
void redirect_standard_out(int pipe_write_end);
void close_pipe(int pipe[2]);
