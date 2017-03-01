#include "ThreadPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Thunk : In computer programming, a thunk is a subroutine that is created, often automatically,
   to assist a call to another subroutine. Thunks are primarily used to represent an additional
   calculation that a subroutine needs to execute, or to call a routine that does not support the
   usual calling mechanism. http://en.wikipedia.org/wiki/Thunk */
typedef struct {
	ThreadPool *pThreadPool; // pointeur sur l'objet ThreadPool
	int ThreadNum; // Numéro du thread, de 0 à n
} threadArg;

void *Thunk(void *arg) {
	threadArg *pThreadArg = (threadArg *)arg;
	ThreadPool *pThreadPool;
	pThreadPool = static_cast<ThreadPool*>(pThreadArg->pThreadPool);
	pThreadPool->MyThreadRoutine(pThreadArg->ThreadNum);
}


/* void ThreadPool(unsigned int nThread)
 Ce constructeur doit initialiser le thread pool. En particulier, il doit initialiser les variables
 de conditions et mutex, et démarrer tous les threads dans ce pool, au nombre spécifié par nThread.
 IMPORTANT! Vous devez initialiser les variables de conditions et le mutex AVANT de créer les threads
 qui les utilisent. Sinon vous aurez des bugs difficiles à comprendre comme des threads qui ne débloque
 jamais de phtread_cond_wait(). */
ThreadPool::ThreadPool(unsigned int nThread) {
	// Cette fonction n'est pas complète! Il vous faut la terminer!
	// Initialisation des membres
    this->nThreadActive = nThread;
    this->bufferValide = true;
    this->buffer = 0;
	// Initialisation du mutex et des variables de conditions.
    pthread_mutex_init(&this->mutex,0);
    this->PoolDoitTerminer = false;
    pthread_cond_init(&this->CondProducteur, 0);
    pthread_cond_init(&this->CondThreadRienAFaire, 0);

	// Création des threads. Je vous le donne gratuit, car c'est un peu plus compliqué que vu en classe.
	pTableauThread        = new pthread_t[nThread];
	threadArg *pThreadArg = new threadArg[nThread];
	int i;
	for (i=0; i < nThread; i++) {
		pThreadArg[i].ThreadNum = i;
		pThreadArg[i].pThreadPool = this;
    	printf("ThreadPool(): en train de creer thread %d\n",i);
   		int status = pthread_create(&pTableauThread[i], NULL, Thunk, (void *)&pThreadArg[i]);
   		if (status != 0) {
   			printf("oops, pthread a retourne le code d'erreur %d\n",status);
   			exit(-1);
    	}
    }
}

/* Destructeur ThreadPool::~ThreadPool()
   Ce destructeur doit détruire les mutex et variables de conditions. */
ThreadPool::~ThreadPool() {
	// À compléter
   pthread_cond_destroy(&this->CondProducteur);
   pthread_cond_destroy(&this->CondThreadRienAFaire);
   pthread_mutex_destroy(&this->mutex);
}

/* void ThreadPool::MyThreadRoutine(int myID)
   Cette méthode est celle qui tourne pour chacun des threads crées dans le constructeur, et qui est
   appelée par la fontion thunk. Cette méthode est donc effectivement le code du thread consommateur,
   qui ne doit quitter qu’après un appel à la méthode Quitter(). Si le buffer est vide, MyThreadRoutine
   doit s'arrêter (en utilisant une variable de condition). Le travail à accomplir est un sleep() d'une
   durée spécifiée dans le buffer.
   */
void ThreadPool::MyThreadRoutine(int myID) {
	// À compléter
    unsigned int temp;
    printf("Thread %d commence!\n", myID);
    while(!this->PoolDoitTerminer){
        pthread_mutex_lock(&this->mutex);
        while(!this->bufferValide){
        //printf("waiting condRienAFaire\n");
        pthread_cond_wait(&this->CondThreadRienAFaire, &this->mutex);
        }
        temp = this->buffer;
        this->buffer = 0;
        this->bufferValide = false;
        printf("Thread %d récupère l'item %d !\n", myID, temp);
        printf("Thread %d va dormir %d sec.\n", myID, temp);
        pthread_cond_signal(&this->CondProducteur);
        pthread_mutex_unlock(&this->mutex);
        sleep(temp);

    }
    printf("############ Thread %d termine!###########\n", myID);
    this->nThreadActive--;
    pthread_exit(0);

}

/* void ThreadPool::Inserer(unsigned int newItem)
   Cette méthode est appelée par le thread producteur pour mettre une tâche à exécuter dans le buffer
   (soit le temps à dormir pour un thread). Si le buffer est marqué comme plein, il faudra dormir
   sur une variable de condition. */
void ThreadPool::Inserer(unsigned int newItem) {
	// À compléter
	pthread_mutex_lock(&this->mutex);
	while(this->bufferValide){
	//printf("waiting condProd\n");
	        pthread_cond_wait(&this->CondProducteur, &this->mutex);
	}
	this->buffer = newItem;
	this->bufferValide = true;
	pthread_cond_signal(&this->CondThreadRienAFaire);
    pthread_mutex_unlock(&this->mutex);
}

/* void ThreadPool::Quitter()
   Cette fonction est appelée uniquement par le producteur, pour indiquer au thread pool qu’il n’y
   aura plus de nouveaux items qui seront produits. Il faudra alors que tous les threads terminent
   de manière gracieuse. Cette fonction doit bloquer jusqu’à ce que tous ces threads MyThreadRoutine
   terminent, incluant ceux qui étaient bloqués sur une variable de condition. */
void ThreadPool::Quitter() {
	// À compléter
    this->PoolDoitTerminer = true;
    pthread_cond_broadcast(&this->CondProducteur);
    pthread_cond_broadcast(&this->CondThreadRienAFaire);
    for(int i= this->nThreadActive - 1; i >= 0; i--){
        //pthread_cond_broadcast(&this->CondThreadRienAFaire);
        pthread_join(pTableauThread[i], 0);
    }
}

