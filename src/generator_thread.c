#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "generator_thread.h"
#include "global_vars.h"
#include "process_thread.h"

void *generate_processes(void *args) {
  pthread_t *tid_arr = malloc(allp * sizeof(pthread_t));
  thread_args *pt_args = malloc(allp * sizeof(thread_args));

  if (inmode == COMMAND) {
    srand(time(NULL));

    // generate processes
    for (int i = 0; i < allp; i++) {
      // generate priority
      int prio = rand() % (max_prio - min_prio + 1) + min_prio;

      // generate process length
      int pl = 0;
      if (!strcmp(dist_pl, "fixed")) {
        pl = avg_pl;
      } else if (!strcmp(dist_pl, "uniform")) {
        pl = rand() % (max_pl - min_pl + 1) + min_pl;
      } else if (!strcmp(dist_pl, "exponential")) {
        double lambda = 0, u = 0;
        do {
          lambda = 1.0 / avg_pl;
          u = rand() / ((double)RAND_MAX + 1);
          pl = ((-1) * log(1 - u)) / lambda;
        } while (pl < min_pl || pl > max_pl);
      } else {
        fprintf(stderr, "distPL is unrecognized\n");
        exit(1);
      }

      // generate interarrival time
      int iat = 0;
      if (!strcmp(dist_iat, "fixed")) {
        iat = avg_iat;
      } else if (!strcmp(dist_iat, "uniform")) {
        iat = rand() % (max_iat - min_iat + 1) + min_iat;
      } else if (!strcmp(dist_iat, "exponential")) {
        double lambda = 0, u = 0;
        do {
          lambda = 1.0 / avg_iat;
          u = rand() / ((double)RAND_MAX + 1);
          iat = ((-1) * log(1 - u)) / lambda;
        } while (iat < min_iat || iat > max_iat);
      } else {
        fprintf(stderr, "distIAT is unrecognized\n");
        exit(1);
      }

      // setting process thread argument struct
      pt_args[i].pid = i + 1;
      pt_args[i].pl = pl;
      pt_args[i].prio = prio;

      // create process thread
      pthread_t tid; // process thread will access it via pthread_self()
      pthread_create(&tid, NULL, (void *)process_thread_execute,
                     (void *)&(pt_args[i]));
      tid_arr[i] = tid;

      if (i < allp - 1) { // do not sleep after creating last process
        int condition = 0;
        do {
          usleep(iat * 1000);

          /* checking if the rqueue is full */
          // lock is necessary, rqsize is shared
          pthread_mutex_lock(&lock_rqueue);
          condition = rq_cur_size >= rqlen;
          pthread_mutex_unlock(&lock_rqueue);
        } while (condition);
      }
    }
  } else if (inmode == INFILE) {
    FILE *fp = fopen((const char *)args, "r"); // args is infile
    if (!fp) {
      fprintf(stderr, "File could not be opened for reading");
    }
    char buf[4]; // to synchronize fscanf format
    int iat;

    // generate processes
    for (int i = 0; i < allp; i++) {
      // reading PL line from file
      fscanf(fp, "%s %d %d", buf, &(pt_args[i].pl), &(pt_args[i].prio));

      pt_args[i].pid = i + 1;
      pthread_t tid; // process thread will access it via pthread_self()
      pthread_create(&tid, NULL, (void *)process_thread_execute,
                     (void *)&(pt_args[i]));
      tid_arr[i] = tid;

      if (i < allp - 1) { // do not sleep after creating last process
        fscanf(fp, "%s %d", buf, &iat); // reading IAT line from file
        int condition = 0;
        do {
          usleep(iat * 1000);

          /* checking if the rqueue is full */
          // lock is necessary, rqsize is shared
          pthread_mutex_lock(&lock_rqueue);
          condition = rq_cur_size >= rqlen;
          pthread_mutex_unlock(&lock_rqueue);
        } while (condition);
      }
    }
    fclose(fp);
  }

  for (int i = 0; i < allp; i++) {
    pthread_join(tid_arr[i], NULL);
  }

  free(pt_args);
  free(tid_arr);
  pthread_exit(0);
}
