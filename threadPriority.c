#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/syscall.h>

#define N_THREADS 5

typedef struct {
int name;
int priority;
}ThreadParam;

void  *setPriority(void *arg) {
  ThreadParam *param = (ThreadParam *)arg;

  printf("Thread # %d.\n", param->name);
  printf("Setting priority to: %d.\n", param->priority);
  
  ThreadID = syscall(SYS_gettid);
  int ret = setpriority(PRIO_PROCESS, ThreadID, param->priority);
}

int main(int argc, char *argv[]) {
  pthread_t threads[N_THREADS];
  ThreadParam params[N_THREADS];

  for (int i=0; i < N_THREADS; i++) {
    params[i].name = i;
    params[i].priority = 0;
    //params[i].priority = i;
    //params[i].priority = 2*i;
    //params[i].priority = ((2*i)-4);

    pthread_create(&threads[i], NULL, setPriority, (void *)&params[i]);
  }  

  for (int i=0; i < N_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  
  exit(0);
}

