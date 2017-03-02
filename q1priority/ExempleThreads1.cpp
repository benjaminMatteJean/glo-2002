#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <pthread.h>


// Pour compiler le fichier : g++ ExempleThreads1.cpp -o ExempleThreads1 -lpthread
#define N_THREADS 5

void  *print_hello(void *tid) {
  printf("Bonjour je suis le thread %d\n",*(int*)tid);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  // Le programme cree 5 threads et se termine
  pthread_t threads[N_THREADS];
  int Arg[N_THREADS];
  int status, i;
  for (i=0; i < N_THREADS; i++) {
    printf("Main(): en train de creer thread %d\n",i);
    Arg[i] = i;
    status = pthread_create(&threads[i], NULL, print_hello, &Arg[i]);
    if (status != 0) {
      printf("oops, pthread a retourne le code d'erreur %d\n",status);
      exit(-1);
    }
    //pthread_join(threads[i], NULL);  // #### LIGNE OPTION #####
  }
  sleep(1); // Attendre une seconde
  exit(NULL);
}

