#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <pthread.h>


// Pour compiler le fichier : g++ ExempleThreads2.cpp -o ExempleThreads2 -lpthread
#define N_THREADS 5
#define N_SPINS  100000000
void  *CalculIntense(void *tid) {
  int ThreadNum = *(int *) tid;
  printf("Thread %d : debut de calcul intense.\n",ThreadNum);
  volatile long int x;
  for (x = 0; x< N_SPINS; x++);
  printf("Thread %d : FIN de calcul intense.\n",ThreadNum);
  pthread_exit(NULL);
}

void  *CalculAvecYield(void *tid) {
  volatile long int x;
  int iteration;
  int ThreadNum = *(int *) tid;
  for (iteration = 0; iteration < 5; iteration++) {
    printf("   Thread %d : iteration %d\n",ThreadNum,iteration);
    for (x = 0; x< (N_SPINS/5); x++);
    pthread_yield();
    printf("   Thread %d : cede l'iteration %d\n",ThreadNum,iteration);
  }
  printf("   Thread %d : FIN de calcul.\n",ThreadNum); 
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
    if (i > 4) {
        status = pthread_create(&threads[i], NULL, CalculIntense, &Arg[i]);
    } else {
        status = pthread_create(&threads[i], NULL, CalculAvecYield, &Arg[i]);
    }
    if (status != 0) {
      printf("oops, pthread a retourne le code d'erreur %d\n",status);
      exit(-1);
    }
  }
  for (i=0; i < N_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  sleep(1); // Attendre une seconde
  exit(NULL);
}

