#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "generator_thread.h"
#include "global_vars.h"
#include "scheduler_thread.h"

void read_args(int argc, char *argv[]) {
  if (inmode == COMMAND) {
    min_prio = atoi(argv[2]);
    max_prio = atoi(argv[3]);
    strcpy(dist_pl, argv[4]);
    avg_pl = atoi(argv[5]);
    min_pl = atoi(argv[6]);
    max_pl = atoi(argv[7]);
    strcpy(dist_iat, argv[8]);
    avg_iat = atoi(argv[9]);
    min_iat = atoi(argv[10]);
    max_iat = atoi(argv[11]);
    rqlen = atoi(argv[12]);
    allp = atoi(argv[13]);
    outmode = atoi(argv[14]);

    // outfile is optional
    if (argc >= 16) {
      outfile = argv[15];
    }

  } else if (inmode == INFILE) {
    rqlen = atoi(argv[2]);
    allp = atoi(argv[3]);
    outmode = atoi(argv[4]);
    infile = argv[5];

    // outfile is optional
    if (argc >= 7) {
      outfile = argv[6];
    }
  }
}

void simulate() {
  // make stdout to point outfile, if specified
  if (outfile) {
    freopen(outfile, "w", stdout);
  }
  rqueue = NULL;
  rq_cur_size = 0;
  pthread_mutex_init(&lock_rqueue, NULL);
  pthread_mutex_init(&lock_cpu, NULL);
  pthread_mutex_init(&lock_count_terminated, NULL);

  pthread_mutex_init(&lock_scheduler_thread, NULL);
  pthread_cond_init(&cond_scheduler_thread, NULL);

  output_info_arr = malloc(allp * sizeof(process_thread_info));

  // measuring simulation start time
  struct timeval start;
  gettimeofday(&start, NULL);
  start_ms = start.tv_sec * 1000 + start.tv_usec / 1000;

  pthread_t id_generator_thread;
  pthread_create(&id_generator_thread, NULL, (void *)generate_processes,
                 (void *)infile);

  pthread_t id_scheduler_thread;
  pthread_create(&id_scheduler_thread, NULL, (void *)schedule, NULL);

  pthread_join(id_generator_thread, NULL);
  pthread_join(id_scheduler_thread, NULL);

  // outputting the measurements
  long sum_waitr = 0;
  printf("\npid      arv     dept     prio      cpu    waitr    turna       "
         "cs\n");
  for (int i = 0; i < allp; i++) {
    process_thread_info output_info = output_info_arr[i];
    printf("%3d %8d %8d %8d %8d %8d %8d %8d\n", output_info.pid,
           output_info.arv, output_info.dept, output_info.prio, output_info.cpu,
           output_info.waitr, output_info.turna, output_info.cs);

    sum_waitr += output_info.waitr;
  }
  printf("avg waiting time: %ld\n", sum_waitr / allp);
}

void clean() {
  pthread_mutex_destroy(&lock_rqueue);
  pthread_mutex_destroy(&lock_cpu);
  pthread_mutex_destroy(&lock_count_terminated);
  pthread_mutex_destroy(&lock_scheduler_thread);
  pthread_cond_destroy(&cond_scheduler_thread);

  free(output_info_arr);

  if (outfile) {
    fclose(stdout);
  }
}

int main(int argc, char *argv[]) {
  if (!strcmp(argv[1], "C")) {
    inmode = COMMAND;
  } else if (!strcmp(argv[1], "F")) {
    inmode = INFILE;
  } else {
    fprintf(stderr, "First argument must be either C or F");
  }

  read_args(argc, argv);
  simulate();
  clean();

  return 0;
}
