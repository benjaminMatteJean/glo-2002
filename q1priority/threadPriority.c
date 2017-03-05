#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#include <string.h>

#define N_THREADS 5

typedef struct {
int numero;
int priority;
}ThreadParam;

pthread_mutex_t mutex;

/** Fonction exécutée par chaque thread pour modifier leur priorité **/
void  *ThreadSetPriority(void *arg) {

  pthread_mutex_lock(&mutex);
  ThreadParam *param = (ThreadParam *)arg;

  /** Affichage d'information sur le thread courant (numero, priorité actuelle, priorité désirée) **/
  printf("========== \nThread #%d \n\n", param->numero);
  printf("Priorité actuelle: %d\n", getpriority(PRIO_PROCESS, syscall(SYS_gettid)));
  printf("Set de la priorité à %d...\n", param->priority);

  /** On change la priorité par celle en argument **/
  int ret = setpriority(PRIO_PROCESS, syscall(SYS_gettid), param->priority);

  /** Si setpriority() a réussi **/
  if(ret==0)
  {
    printf("Priorité changée avec succès pour: %d\n", getpriority(PRIO_PROCESS, syscall(SYS_gettid)));
    printf("setPriority() returne le code: %d\n", ret);
    printf("Valeur de errno: %d\n", errno);
  }

  /** Si setpriority() a échoué **/
  else
  {
    printf("CHANGEMENT DE PRIORITÉ ÉCHOUÉ.\n");
    printf("setPriority() returne le code: %d\n", ret);
    printf("Valeur de errno: %d\n", errno);
  }

  pthread_mutex_unlock(&mutex);
}

/** Programme Principal **/
int main(int argc, char *argv[]) {
  pthread_t threads[N_THREADS];
  ThreadParam params[N_THREADS];

  for (int i=0; i < N_THREADS; i++) {
    params[i].numero = i;
    /** Les lignes suivantes sont les différentes combinaisons de priorités demandées **/
    //params[i].priority = 0;
    //params[i].priority = i;
    //params[i].priority = 2*i;
    params[i].priority = ((2*i)-4);
    
    /** Affichage et création du thread courant.
    /** La fonction exécutée lors de la création de chaque thread est protégée par un mutex. **/
    printf("Création du thread # %d...\n", params[i].numero);
    pthread_create(&threads[i], NULL, ThreadSetPriority, (void *)&params[i]);
  }  

  for (int i=0; i < N_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  
  exit(0);
}

