#include <stdio.h>
#include <pthread.h>
#define N_ITER 1000000
int count = 0; // variables globales (donc partagées entre threads)
const int ModuloPrint = 100;
int critical[2] = {0,0}; 


// Mauvais Peterson (comme dans les acétates du cours)
void enter_region(int process) {
	int other;
	other = 1 - process;
	critical[process]=1;
	while (critical[other] == 1);
} 

void leave_region(int process) {
	critical[process] = 0;
}

void *CodeThread(void * a) {
    int i, tmp;
    int ThreadNum = *(int *)a;
    for(i = 0; i < N_ITER; i++)     {
	    enter_region(ThreadNum);
        tmp = count;      // copie locale de variable globale
        tmp = tmp + 1;    // incrémente la copie locale
        count = tmp;      // écrit dans globale 
        /* La vitesse d'execution diminue si printf est dans la region critique. L'operation modulo (%)
           dans la condition if permet d'imprimer seulement periodiquement, ce qui augmente les chances
           d'interblocage selon mes tests. */        
        if (count%ModuloPrint==0) {
        	printf("%d ", count); 
        	fflush(stdout); // Pour imprimer immediatement a l'ecran, car printf est bufferise.
        }
	    leave_region(ThreadNum);
    }
}

int main(int argc, char **argv) {
   pthread_t Count1, Count2;
   int ThreadNum[2] = {0, 1};
   pthread_create(&Count1, 0, CodeThread, &ThreadNum[0]);
   pthread_create(&Count2, 0, CodeThread, &ThreadNum[1]);
   pthread_join(Count2, 0);
   pthread_join(Count1, 0);
   printf("Le total est %d\n",count);
}


       
       
