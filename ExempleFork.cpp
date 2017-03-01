#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

// Pour compiler le fichier : g++ ExempleFork.cpp -o ExempleFork
int main ( int argc, char *argv[ ] ) { 
    pid_t pid, pid_parent, pid_fils, pid_val; 
    int status;
    pid_parent = getpid();
    printf("PARENT: Processus parent est %d\n",pid_parent);
    pid_val = wait(&status);
    printf("PARENT: N'a pas attendu, car aucun enfant. PID retour=%d\n",pid_val);
    pid = fork();
    if (pid == 0)
    { 	/* code exécuté par le processus fils */
 
        int i;
        pid_fils = getpid(); 
	    printf("   FILS: Processus fils est %d\n",pid_fils); 
	    for (i = 5; i > 0; i--) {
	       printf("   FILS: Decompte: %d...\n",i);
	       sleep(1);
	       // int *pRandom = 0x000000; (*pRandom) = 0;
	    }
	    // execl( "/usr/bin/ps", "ps", 0 );
        printf("   FILS: Termine l'execution.\n");
    }
    else {
       /* Code exectue par le parent seulement */
       pid_fils = pid;
       printf("PARENT: En attente sur le processus fils, qui a le PID=%d\n",pid_fils);
       pid_val = wait(&status);
       printf("PARENT: Fin de l'attente sur le processus fils %d\n",pid_val);
       return 0;
    }
 }
