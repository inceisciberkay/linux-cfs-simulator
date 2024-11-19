#ifndef SCHEDULER_THREAD_H_
#define SCHEDULER_THREAD_H_

#include <pthread.h>

void *schedule(void *);

int is_scheduling_needed();

int is_simulation_finished();

#endif