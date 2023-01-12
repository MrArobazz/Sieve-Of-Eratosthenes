#ifndef MASTER_WORKER_H
#define MASTER_WORKER_H

// On peut mettre ici des éléments propres au couple master/worker :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (écriture dans un tube, ...)

/********************************************/
/*          PROJET PROG AVANCEE EN C        */ 
/*     AL NATOUR MAZEN && CAILLAUD TOM      */           
/********************************************/

#define ORDER_STOP               -1
#define SUCCESS					  1 
#define FAILURE					  0
#define NoNextWorker             -2




void create_workers(int number, int fdRead, int fdWrite); 		// operation permettant de creer les prochains workers

#endif
