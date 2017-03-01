// =============================================
//    Code pour le TP2, question3 de GLO-2001
//                Hiver 2015
//         (c) Philippe Giguere, 2016
// =============================================
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <sys/timeb.h>
#include "ThreadPool.h"

// Pour compiler, faire make dans le répertoire.
int main(int argc, char *argv[]) {
	// Extraction des paramètres d'entrées
	if (argc != 3) {
		printf("Il n'y a pas les deux paramètres d'entrées!\n");
		printf("Usage : q3threadpool nombre_threads nombre_items\n");
		exit(-1);
	}
	int nThreads = atoi(argv[1]); // Nombre de threads dans le thread pool
	int maxItems = atoi(argv[2]); // Nombre d'items maximum a produire
	printf("Programme de test avec %d threads et %d items.\n",nThreads,maxItems);

	// Structure de temps pour pouvoir faire afficher un timestamp des messages, afin
	// de faciliter la correction/visualisation du comportement.
	struct timeb StartTime,actualTime;

	// Declaration/création du thread pool
	ThreadPool MonThreadPool(nThreads);
	ftime(&StartTime);	// On prend note du temps de départ

	int i;
	for (i = 0; i < maxItems; i++) {
		ftime(&actualTime); // On prend note du temps actuel
		unsigned int sleepTime = (i%4)+1; // On pige un entier au hasard, entre 1 et 3.
		printf("(%ld.%03d) main: Je produis item numero %d avec valeur %d.\n",
		        actualTime.time-StartTime.time,actualTime.millitm,i,sleepTime);
		MonThreadPool.Inserer(sleepTime);
		printf("           main: item inséré.\n");
	}

	/* Comme le producteur a terminé, nous devons indiquer au thread pool de se terminer lui aussi.
	   Cette méthode Quitter() doit bloquer jusqu'à la fin de la destruction de tous les threads. */
	ftime(&actualTime);
	printf("(%ld.%03d) main: Destruction du thread pool.\n",
		        actualTime.time-StartTime.time,actualTime.millitm);
	MonThreadPool.Quitter(); // Fonction bloquante
	ftime(&actualTime);
	printf("(%ld.%03d) main: FIN!\n",
		        actualTime.time-StartTime.time,actualTime.millitm);
	// Le programme peut maintenant se terminer
  	exit(NULL);
}

