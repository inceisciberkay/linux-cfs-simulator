#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "global_vars.h"
#include "process_thread.h"

#define SCHED_LATENCY 100
#define MIN_GRANULARITY 10

void *process_thread_execute(void *args) {
  thread_args *pt_args = (thread_args *)args;
  process_thread_info measurements = {0, 0, 0, 0, 0, 0, 0, 0};
  struct timeval in_rq, out_rq; // for waiting time measurement
  long in_rq_ms, out_rq_ms;

  // recording arrival time
  struct timeval arv;
  gettimeofday(&arv, NULL);
  long arv_ms = arv.tv_sec * 1000 + arv.tv_usec / 1000;
  measurements.arv = arv_ms - start_ms;

  pthread_cond_t cond_process_thread;
  pthread_cond_init(&cond_process_thread, NULL);
  pthread_mutex_t lock_process_thread;
  pthread_mutex_init(&lock_process_thread, NULL);

  // creating PCB
  PCB pcb = {.pid = pt_args->pid,
             .tid = pthread_self(),
             .prio = pt_args->prio,
             .pl = pt_args->pl,
             .actual_cpu_time = 0,
             .virtual_runtime = 0.0,
             .cond = &cond_process_thread};
  strcpy(pcb.state, "READY");

  if (outmode == PRINT_VERBOSE) {
    printf("%ld.ms --> PROCESS %d CREATED! pid: %d, tid: %ld, prio: %d, "
           "pl: %d\n",
           arv_ms - start_ms, pcb.pid, pcb.pid, pcb.tid, pcb.prio, pcb.pl);
    fflush(stdout);
  }

  // saving pid and prio
  measurements.pid = pcb.pid;
  measurements.prio = pcb.prio;

  // adding pcb to runqueue and record time
  pthread_mutex_lock(&lock_rqueue);
  ll_insert(&rqueue, &pcb);
  gettimeofday(&in_rq, NULL);
  in_rq_ms = in_rq.tv_sec * 1000 + in_rq.tv_usec / 1000;
  pthread_mutex_unlock(&lock_rqueue);

  if (outmode == PRINT_VERBOSE) {
    printf("%ld.ms --> PROCESS %d ADDED TO RQ! pid: %d, prio: %d\n",
           in_rq_ms - start_ms, pcb.pid, pcb.pid, pcb.prio);
    fflush(stdout);
  }

  // process thread is created and added to the runqueue
  // signaling the scheduler thread
  pthread_mutex_lock(&lock_scheduler_thread);
  pthread_cond_signal(&cond_scheduler_thread);
  pthread_mutex_unlock(&lock_scheduler_thread);

  pthread_mutex_lock(&lock_process_thread);
  while (pcb.actual_cpu_time < pcb.pl) {
    // sleeping on its condition variable
    pthread_cond_wait(&cond_process_thread, &lock_process_thread);

    if (outmode == PRINT_VERBOSE) {
      struct timeval selected_time;
      gettimeofday(&selected_time, NULL);
      long selected_time_ms =
          selected_time.tv_sec * 1000 + selected_time.tv_usec / 1000;
      printf("%ld.ms --> PROCESS %d IS SELECTED BY THE SCHEDULER! pid: %d, "
             "prio: %d, "
             "pl: %d\n",
             selected_time_ms - start_ms, pcb.pid, pcb.pid, pcb.prio, pcb.pl);
      fflush(stdout);
    }

    // - calculating timeslice and remaining time (ms) to determine runtime
    // - removing pcb from the runqueue
    pthread_mutex_lock(&lock_rqueue);
    llnode *ptr = rqueue;
    double sum_weight = 0;
    while (ptr) {
      sum_weight += prio_to_weight[ptr->ptr_pcb->prio + 20];
      ptr = ptr->next;
    }
    int timeslice =
        ((prio_to_weight[pcb.prio + 20]) * SCHED_LATENCY) / sum_weight;

    if (timeslice < MIN_GRANULARITY)
      timeslice = MIN_GRANULARITY;

    int remaining_time = pcb.pl - pcb.actual_cpu_time;

    int runtime = timeslice < remaining_time ? timeslice : remaining_time;

    ll_remove(&rqueue, &pcb);
    // process is removed from the rqueue, record waiting time
    gettimeofday(&out_rq, NULL);
    out_rq_ms = out_rq.tv_sec * 1000 + out_rq.tv_usec / 1000;
    measurements.waitr += (out_rq_ms - in_rq_ms);
    pthread_mutex_unlock(&lock_rqueue);

    if (outmode == PRINT_VERBOSE) {
      printf("%ld.ms --> PROCESS %d IS REMOVED FROM RQ! pid: %d, prio: %d, "
             "pl: %d\n",
             out_rq_ms - start_ms, pcb.pid, pcb.pid, pcb.prio, pcb.pl);
      fflush(stdout);
    }

    // changing the state from READY to RUNNING
    strcpy(pcb.state, "RUNNING");

    // setting global variable: pcb of currently running process
    pthread_mutex_lock(&lock_cpu);
    pcb_of_process_in_cpu = &pcb;
    pthread_mutex_unlock(&lock_cpu);

    // printing status
    if (outmode == PRINT_MINIMAL) {
      printf("%ld %d %s %d\n", out_rq_ms - start_ms, pcb.pid, pcb.state,
             runtime);
      fflush(stdout);
    }

    if (outmode == PRINT_VERBOSE) {
      struct timeval start_exec_time;
      gettimeofday(&start_exec_time, NULL);
      long start_exec_time_ms =
          start_exec_time.tv_sec * 1000 + start_exec_time.tv_usec / 1000;
      printf("%ld.ms --> PROCESS %d IS RUNNING IN THE CPU FOR %d ms! pid: "
             "%d, prio: %d, pl: %d, remaining_time: %d ms\n",
             start_exec_time_ms - start_ms, pcb.pid, runtime, pcb.pid, pcb.prio,
             pcb.pl, remaining_time);
      fflush(stdout);
    }

    /* EXECUTE IN CPU */
    usleep(runtime * 1000);

    // process underwent a context switch
    measurements.cs++;

    // updating pcb actual cpu runtime
    pcb.actual_cpu_time += runtime;
    measurements.cpu += runtime;

    // updating pcb virtual runtime, weight of nice value 0 is
    // prio_to_weight[20]
    pcb.virtual_runtime +=
        ((double)prio_to_weight[20] / prio_to_weight[pcb.prio + 20]) *
        (double)runtime;

    if (outmode == PRINT_VERBOSE) {
      struct timeval expire_timeslice_time;
      gettimeofday(&expire_timeslice_time, NULL);
      long expire_timeslice_time_ms = expire_timeslice_time.tv_sec * 1000 +
                                      expire_timeslice_time.tv_usec / 1000;
      printf("%ld.ms --> PROCESS %d FINISHED ITS TIMESLICE! pid: %d, "
             "prio: %d, "
             "pl: %d, virtual_runtime: %lf ms\n",
             expire_timeslice_time_ms - start_ms, pcb.pid, pcb.pid, pcb.prio,
             pcb.pl, pcb.virtual_runtime);
      fflush(stdout);
    }

    // setting global variable: pcb of currently running process to NULL
    pthread_mutex_lock(&lock_cpu);
    pcb_of_process_in_cpu = NULL;
    pthread_mutex_unlock(&lock_cpu);

    // changing the state from RUNNING to READY
    strcpy(pcb.state, "READY");

    // timeslice expired, add pcb to the runqueue again, and record time
    // do not add the process to rqueue if it finished its execution
    if (pcb.actual_cpu_time < pcb.pl) {
      pthread_mutex_lock(&lock_rqueue);
      ll_insert(&rqueue, &pcb);
      gettimeofday(&in_rq, NULL);
      in_rq_ms = in_rq.tv_sec * 1000 + in_rq.tv_usec / 1000;
      pthread_mutex_unlock(&lock_rqueue);

      if (outmode == PRINT_VERBOSE) {
        printf("%ld.ms --> PROCESS %d ADDED TO RQ! pid: %d, prio: %d, "
               "pl: %d\n",
               in_rq_ms - start_ms, pcb.pid, pcb.pid, pcb.prio, pcb.pl);
        fflush(stdout);
      }
    }

    // signaling the scheduler thread
    pthread_mutex_lock(&lock_scheduler_thread);
    pthread_cond_signal(&cond_scheduler_thread);
    pthread_mutex_unlock(&lock_scheduler_thread);
  }
  pthread_mutex_unlock(&lock_process_thread);

  // process thread terminates, signaling the scheduler thread
  pthread_mutex_lock(&lock_scheduler_thread);
  pthread_cond_signal(&cond_scheduler_thread);
  pthread_mutex_unlock(&lock_scheduler_thread);

  // incrementing number of terminated processes
  pthread_mutex_lock(&lock_count_terminated);
  no_of_terminated_processes++;
  pthread_mutex_unlock(&lock_count_terminated);

  // destroying condition variable and lock
  pthread_mutex_destroy(&lock_process_thread);
  pthread_cond_destroy(&cond_process_thread);

  // recording departure time
  struct timeval dept;
  gettimeofday(&dept, NULL);
  long dept_ms = dept.tv_sec * 1000 + dept.tv_usec / 1000;
  measurements.dept = dept_ms - start_ms;

  if (outmode == PRINT_VERBOSE) {
    printf("%ld.ms --> PROCESS %d TERMINATES! pid: %d, prio: %d, "
           "pl: %d\n",
           dept_ms - start_ms, pcb.pid, pcb.pid, pcb.prio, pcb.pl);
    fflush(stdout);
  }

  // calculating turnaround time
  measurements.turna = dept_ms - arv_ms;

  // save the measurements to display it later
  output_info_arr[measurements.pid - 1] = measurements;

  pthread_exit(0);
}
