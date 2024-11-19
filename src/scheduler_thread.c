#include <pthread.h>
#include <stdio.h>

#include "global_vars.h"
#include "scheduler_thread.h"

void *schedule(void *args) {
  pthread_mutex_lock(&lock_scheduler_thread);
  while (!is_simulation_finished()) {
    pthread_cond_wait(&cond_scheduler_thread, &lock_scheduler_thread);
    // lock cpu and rqueue
    pthread_mutex_lock(&lock_cpu);
    pthread_mutex_lock(&lock_rqueue);

    // check if scheduling is needed
    if (is_scheduling_needed()) {
      /* perform scheduling: traverse runqueue, find a process thread with
       * lowest virtual runtime, and signal its condition variable */
      PCB *ptr_pcb_with_smallest_vruntime = rqueue->ptr_pcb;
      llnode *ptr = rqueue->next;

      while (ptr) {
        // select the process with the lowest vruntime
        if (ptr->ptr_pcb->virtual_runtime <
            ptr_pcb_with_smallest_vruntime->virtual_runtime) {
          ptr_pcb_with_smallest_vruntime = ptr->ptr_pcb;
        }
        // if virtual runtimes are equal, select the one with higher
        // priority
        else if (ptr->ptr_pcb->virtual_runtime ==
                     ptr_pcb_with_smallest_vruntime->virtual_runtime &&
                 ptr->ptr_pcb->prio >= ptr_pcb_with_smallest_vruntime->prio) {
          ptr_pcb_with_smallest_vruntime = ptr->ptr_pcb;
        }
        ptr = ptr->next;
      }
      pthread_cond_signal(ptr_pcb_with_smallest_vruntime->cond);
    }
    // unlock cpu and rqueue
    pthread_mutex_unlock(&lock_cpu);
    pthread_mutex_unlock(&lock_rqueue);
  }
  pthread_mutex_unlock(&lock_scheduler_thread);
  pthread_exit(0);
}

int is_scheduling_needed() {
  /* check if there is a process thread running in the cpu (if a process
   * thread is consuming its timeslice, it means it is running it the cpu)
   * and if there is a process in the ready queue */
  // rqueue lock and cpu lock are obtained in the calling function
  return pcb_of_process_in_cpu == NULL && rq_cur_size > 0;
}

int is_simulation_finished() {
  int result = 0;
  pthread_mutex_lock(&lock_count_terminated);
  if (no_of_terminated_processes == allp)
    result = 1;
  pthread_mutex_unlock(&lock_count_terminated);
  return result;
}
