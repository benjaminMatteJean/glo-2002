#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>


#define N_THREADS 6

sem_t Semaphore[N_THREADS];

int Ordre[6] = {2,3,1,4,5,0}; // L'ordre dans lequel les threads doivent s'exécuter

typedef struct {
int ThreadNum;
// Vos paramètres supplémentaires (si nécessaire)
}Parametres;

// Vos variables globales supplémentaires  (si nécessaire)

void  *FonctionThread(void *data) {
  long i;
  Parametres *pParam = (Parametres *)data;

   // Votre code ici (si nécessaire)

  printf("Je suis le thread %d.\n", pParam->ThreadNum);

  // Votre code ici (si nécessaire)

  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  pthread_t threads[N_THREADS];
  Parametres myParam[N_THREADS];
  int i,j;

  // Votre code ici (si nécessaire)

  // Vous ne pouvez pas modifier le code en gris
  for (i=0; i < N_THREADS; i++) {
    myParam[i].ThreadNum = i;
    pthread_create(&threads[i], NULL, FonctionThread, (void *)&myParam[i]);
  }  

  for (i=0; i < N_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

   // Votre code ici (si nécessaire)
  
  exit(0);
}

