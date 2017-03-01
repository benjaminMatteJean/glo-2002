#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>

// Code retourne par la methode Inserer
class ThreadPool {
private:
	// Membres pour les threads et la synchronisation
	pthread_t *pTableauThread;
	pthread_mutex_t mutex;
	pthread_cond_t CondThreadRienAFaire;
	pthread_cond_t CondProducteur;
	
	// Membres pour stocker l'état du thread pool et le nombre de threads encore actives
	bool PoolDoitTerminer;  // Mettre ce drapeau à vrai lord de l'appel de la méthode Quitter()
	int  nThreadActive;    

	// Membres pour gérer l'unique buffer contenant l'item à consommer
	bool bufferValide;
	unsigned int buffer;
	
public:
	// Voici les méthodes à implémenter
    void MyThreadRoutine(int myID);
    ThreadPool(unsigned int nThread);
    ~ThreadPool();
	void Inserer(unsigned int newItem);
    void Quitter(void);
};
#endif
