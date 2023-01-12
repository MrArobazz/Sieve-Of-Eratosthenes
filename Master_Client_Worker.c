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
#include "Master_Client_Worker.h"

/********************************************/
/*          PROJET PROG AVANCEE EN C        */ 
/*     AL NATOUR MAZEN && CAILLAUD TOM      */           
/********************************************/

void CreateTube(const char * file, int flags)
{
    int check = mkfifo(file, flags);
    myassert(check != -1, "problème de création tube nommé ");
}

void DestructTube(const char * file)
{
    int check = unlink(file);
    myassert(check != -1, "probleme destruction d'un tube nommé");
}

int OpenTube(const char * file, int flags)
{
    int retour = open(file, flags);
    myassert(retour != -1,"L ouverture du tube a echoue");

    return retour;
}

void CloseTube(int thetube)
{
    int ret_close = close(thetube);
    myassert(ret_close == 0 , "La fermeture du tube a echoue");
}

void WriteData(int thetube, int *data)
{
    int envoie_donnee_ret = write(thetube,data,sizeof(int));
    myassert(envoie_donnee_ret != -1,"L envoie des donnees a echoue");
}

int ReadData(int thetube)
{
    int data = 0;
    int LectureReponse_ret = read(thetube,&data,sizeof(int));
    myassert(LectureReponse_ret != -1,"La Lecture de la reponse a echoue");

    return data;
}