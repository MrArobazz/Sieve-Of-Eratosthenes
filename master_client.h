#ifndef CLIENT_CRIBLE
#define CLIENT_CRIBLE

// On peut mettre ici des éléments propres au couple master/client :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (création tubes, écriture dans un tube,
//      manipulation de sémaphores, ...)

// fichier (qui doit être accessible) choisi pour l'identification du sémaphore
#define MON_FICHIER "master_client.h"

#define MUTEX1_ID 10
#define MUTEX2_ID 5
#define MUTEX3_ID 15

// ordres possibles pour le master
#define ORDER_NONE                0
#define ORDER_STOP               -1
#define ORDER_COMPUTE_PRIME       1
#define ORDER_HOW_MANY_PRIME      2
#define ORDER_HIGHEST_PRIME       3
#define ORDER_COMPUTE_PRIME_LOCAL 4   // ne concerne pas le master

// bref n'hésitez à mettre nombre de fonctions avec des noms explicites
// pour masquer l'implémentation

/********************************************/
/*          PROJET PROG AVANCEE EN C        */ 
/*     AL NATOUR MAZEN && CAILLAUD TOM      */           
/********************************************/

int MySemCreate(int value, int id); 						// operation de creation et d'initialisation d'un semaphore
int MySemGet(int id);										// operation de recuperation d'un semaphore existant
void PrendreSem(int idSem);									// operation de -1 sur un semaphore
void VendreSem(int idSem);									// operation de +1 sur un semaphore
void MySemDestruct(int id);									// operation de destruction sur un semaphore

#endif
