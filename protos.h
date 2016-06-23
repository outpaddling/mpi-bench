/* mpi_bench.c */
int main(int argc, char *argv[]);
int root_process(int argc, char *argv[], int proc_count, int trials);
int non_root_process(int trials);
time_t difftimeofday(struct timeval *later, struct timeval *earlier);
void broadcast(int proc_count);
void usage(char *argv[]);
