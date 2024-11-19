#include "global_vars.h"

enum INMODE inmode;

// common arguments
int rqlen, allp;
enum OUTMODE outmode;
const char *outfile = NULL;

// command mode specific arguments
int min_prio, max_prio;
char dist_pl[16], dist_iat[16];
int avg_pl, min_pl, max_pl, avg_iat, min_iat, max_iat;

// file mode speficic arguments
const char *infile;

// simulation start time
long start_ms;

// rqueue
llnode *rqueue;
int rq_cur_size;
pthread_mutex_t lock_rqueue;

// pcb of currently running process
PCB *pcb_of_process_in_cpu;
pthread_mutex_t lock_cpu;

// scheduler thread will use it to determine whether its job is finished
// generator thread will use it to determine the length of rqueue
// it will be updated by process threads
int no_of_terminated_processes;
pthread_mutex_t lock_count_terminated;

// condition variable for the scheduler thread
pthread_cond_t cond_scheduler_thread;
pthread_mutex_t lock_scheduler_thread;

// array containing output information for each process thread
process_thread_info *output_info_arr;
