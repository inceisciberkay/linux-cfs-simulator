#ifndef GLOBAL_VARS_H_
#define GLOBAL_VARS_H_

#include <sys/time.h>

#include "linked_list.h"

enum INMODE { COMMAND, INFILE };
extern enum INMODE inmode;

// common arguments
extern int rqlen, allp;
enum OUTMODE { NO_PRINT = 1, PRINT_MINIMAL = 2, PRINT_VERBOSE = 3 };
extern enum OUTMODE outmode;
extern const char *outfile;

// command mode specific arguments
extern int min_prio, max_prio;
extern char dist_pl[16], dist_iat[16];
extern int avg_pl, min_pl, max_pl, avg_iat, min_iat, max_iat;

// file mode speficic arguments
extern const char *infile;

// simulation start time
extern long start_ms;

// rqueue
extern llnode *rqueue;
extern int rq_cur_size;
extern pthread_mutex_t lock_rqueue;

// pcb of currently running process
extern PCB *pcb_of_process_in_cpu;
extern pthread_mutex_t lock_cpu;

// scheduler thread will use it to determine whether its job is finished
// generator thread will use it to determine the length of rqueue
// it will be updated by process threads
extern int no_of_terminated_processes;
extern pthread_mutex_t lock_count_terminated;

// condition variable for the scheduler thread
extern pthread_cond_t cond_scheduler_thread;
extern pthread_mutex_t lock_scheduler_thread;

// not pcb, just to pass arguments from generator to process threads
typedef struct {
    int pid;
    int pl;
    int prio;
} thread_args;

// structure for outputing time measurements at the end of the simulation
typedef struct {
    int pid;
    int arv;
    int dept;
    int prio;
    int cpu;
    int waitr;
    int turna;
    int cs;
} process_thread_info;

// array containing output information for each process thread
extern process_thread_info *output_info_arr;

#endif