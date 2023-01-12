#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "myassert.h"

#include "master_client.h"


/********************************************/
/*          PROJET PROG AVANCEE EN C        */ 
/*     AL NATOUR MAZEN && CAILLAUD TOM      */           
/********************************************/

int MySemCreate(int value, int id)
{
	key_t cle;
    int idSem, check;

    cle = ftok(MON_FICHIER, id);
    myassert(cle != -1, "probleme initialisation cle");

    idSem = semget(cle, 1, IPC_CREAT | IPC_EXCL | 0641);
    myassert(idSem != -1, "probleme creation semaphore");

    check = semctl(idSem, 0, SETVAL, value);
    myassert(check != -1, "probleme initialisation semaphore");

    return idSem;
}

int MySemGet(int id)
{
    key_t cle_ret = ftok(MON_FICHIER, id);
    myassert(cle_ret != -1,"La creation de la cle Client a echoue");

    int semId = semget(cle_ret, 1, 0);
    myassert(semId != -1,"L acces au semaphore a echoue");

    return semId;
}

void PrendreSem(int idSem)
{
    struct sembuf opstake = {0,-1,0};
    int restake = semop(idSem,&opstake,1);
    myassert(restake != -1,"L operation de prendre sur un semaphore a echoue");
}

void VendreSem(int idSem)
{
    struct sembuf opswait = {0,1,0};
    int reswait = semop(idSem,&opswait,1);
    myassert(reswait != -1,"L operation de vendre sur un semaphore a echoue");
}

void MySemDestruct(int id)
{
    int check = semctl(id, -1, IPC_RMID);
    myassert(check != -1,"L operation de destruction sur un semaphore a echoue");
}
