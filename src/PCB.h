#ifndef PCB_H_
#define PCB_H_

#include <pthread.h>

enum STATE { READY, RUNNING };

typedef struct PCB {
    int pid;
    pthread_t tid;
    int prio;
    char state[16];
    int pl;
    int actual_cpu_time;
    double virtual_runtime;
    pthread_cond_t *cond;
} PCB;

#endif