#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>



#include "myassert.h"

#include "master_client.h"
#include "master_worker.h"
#include "Master_Client_Worker.h"

/********************************************/
/*          PROJET PROG AVANCEE EN C        */ 
/*     AL NATOUR MAZEN && CAILLAUD TOM      */           
/********************************************/

/************************************************************************
 * Données persistantes d'un master
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le master
// a besoin

typedef struct 
{
    int client_to_master; //tube nommé en lecture vers le client
    int master_to_client; //tube nommé en écriture vers le client
    int mutex_1; //Mutex pour empêcher deux clients de lancer leur commande en même temps
    int mutex_2; //Mutex pour bloquer le master tant que le client n'est pas sorti de la première section critique
    int mutex_3; //Mutex pour bloquer le client tant que le master n'a pas fermé ses tubes
    int masterToWorker[2]; //tube anonyme en ecriture vers le worker
    int workerToMaster[2]; //tube anonyme en lecture vers le worker
} ComDatas;

typedef struct
{
    int check;
    pid_t retFork;
    int orderReceived;
    int numberGiven;
    int highestPrime;
    int occurence;
    int workerAnswer;
    bool notStopped;
} InternDatas;


/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s\n", exeName);
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}


/************************************************************************
 * boucle principale de communication avec le client
 ************************************************************************/
void loop(ComDatas DatasCom, InternDatas DatasInterns)
{
    // boucle infinie :
    // - ouverture des tubes (cf. rq client.c)
    while(DatasInterns.notStopped) 
    { 
        DatasCom.client_to_master = OpenTube("client_to_master", O_RDONLY);
        DatasCom.master_to_client  = OpenTube("master_to_client", O_WRONLY);

        // - attente d'un ordre du client (via le tube nommé)
        DatasInterns.orderReceived = ReadData(DatasCom.client_to_master);
        printf("\n>> J'ai reçu l'ordre = %d\n\n", DatasInterns.orderReceived);

        // - si ORDER_STOP
        if (DatasInterns.orderReceived == ORDER_STOP)
        {
           // . envoyer ordre de fin au premier worker et attendre sa fin
            WriteData(DatasCom.masterToWorker[1], &DatasInterns.orderReceived);

            DatasInterns.check = wait(NULL);
            myassert(DatasInterns.check != -1, "erreur wait du fils");

            //       . envoyer un accusé de réception au client
            WriteData(DatasCom.master_to_client, &DatasInterns.orderReceived);

            DatasInterns.notStopped = false;
       }
       // - si ORDER_COMPUTE_PRIME
       else if (DatasInterns.orderReceived == ORDER_COMPUTE_PRIME)
       {
           //       . récupérer le nombre N à tester provenant du client
            DatasInterns.numberGiven = ReadData(DatasCom.client_to_master);

            printf("\n>> Le nombre recu est = %d\n\n", DatasInterns.numberGiven);
           
            //       . construire le pipeline jusqu'au nombre N-1 (si non encore fait) :
            //             il faut connaître le plus nombre (M) déjà enovoyé aux workers
            //             on leur envoie tous les nombres entre M+1 et N-1
            //             note : chaque envoie déclenche une réponse des workers
            if (DatasInterns.numberGiven > DatasInterns.highestPrime)
            {
                for ( int i = DatasInterns.highestPrime + 1 ; i < DatasInterns.numberGiven ; i++ )
                {
                 
                    WriteData(DatasCom.masterToWorker[1], &DatasInterns.orderReceived);
                    WriteData(DatasCom.masterToWorker[1], &i);
                    DatasInterns.workerAnswer = ReadData(DatasCom.workerToMaster[0]);

                    if (DatasInterns.workerAnswer == SUCCESS)
                    {
                        DatasInterns.highestPrime = i;
                        DatasInterns.occurence ++;
                    }
                }
            }
            //       . envoyer N dans le pipeline
            WriteData(DatasCom.masterToWorker[1], &DatasInterns.orderReceived);
            WriteData(DatasCom.masterToWorker[1], &DatasInterns.numberGiven);
            
            //       . récupérer la réponse
            DatasInterns.workerAnswer = ReadData(DatasCom.workerToMaster[0]);

            if (DatasInterns.workerAnswer == SUCCESS)
            {
                DatasInterns.highestPrime = DatasInterns.numberGiven;
                DatasInterns.occurence ++;
            }
            //       . la transmettre au client
            WriteData(DatasCom.master_to_client, &DatasInterns.workerAnswer);
       }
       // - si ORDER_HOW_MANY_PRIME
       else if (DatasInterns.orderReceived == ORDER_HOW_MANY_PRIME)
       {
       //       . transmettre la réponse au client
            WriteData(DatasCom.master_to_client, &DatasInterns.occurence);
       }
       // - si ORDER_HIGHEST_PRIME
       else
       {
       //       . transmettre la réponse au client
            WriteData(DatasCom.master_to_client, &DatasInterns.highestPrime);
       }

       // - fermer les tubes nommés
      // - attendre ordre du client avant de continuer (sémaphore : Rendez-vous)
       PrendreSem(DatasCom.mutex_2);
       CloseTube(DatasCom.client_to_master);
       CloseTube(DatasCom.master_to_client);
       PrendreSem(DatasCom.mutex_3);

       sleep(1); //Probleme synchro si le master detruit les semaphores avant que le client fasse l'op VendreSem // CF. rapport.pdf

  
    }
}

/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    if (argc != 1)
        usage(argv[0], NULL);

    ComDatas DatasCom;
    InternDatas DatasInterns;

    // - création des sémaphores
    DatasCom.mutex_1 = MySemCreate(1, MUTEX1_ID);
    DatasCom.mutex_2 = MySemCreate(0, MUTEX2_ID);
    DatasCom.mutex_3 = MySemCreate(0, MUTEX3_ID);

    // - création des tubes nommés
    CreateTube("client_to_master", 0644);
    CreateTube("master_to_client", 0644);

    // - création du premier worker
    DatasInterns.check = pipe(DatasCom.masterToWorker);
    myassert(DatasInterns.check == 0, "probleme creation tube anonyme masterToWorker");

    DatasInterns.check = pipe(DatasCom.workerToMaster);
    myassert(DatasInterns.check == 0, "probleme creation tube anonyme workerToMaster");

    DatasInterns.retFork = fork();
    myassert(DatasInterns.retFork != -1, "probleme creation fork fils");

    if (DatasInterns.retFork == 0)
    {
        close(DatasCom.masterToWorker[1]); //on close le tube masterToWorker en ecriture pour le fils
        close(DatasCom.workerToMaster[0]); //on close le tube workerToMaster en lecture pour le fils

        create_workers(2, DatasCom.masterToWorker[0], DatasCom.workerToMaster[1]);
        
        //on est pas censé arriver ici
        close(DatasCom.masterToWorker[0]);
        close(DatasCom.workerToMaster[1]);
        myassert(false, "on est pas censé arriver ici");
    }

    close(DatasCom.masterToWorker[0]); //on close le tube masterToWorker en lecture pour le pere
    close(DatasCom.workerToMaster[1]); //on close le tube workerToMaster en ecriture pour le pere

    DatasInterns.highestPrime = 2;
    DatasInterns.occurence = 1;
    DatasInterns.workerAnswer = false;
    DatasInterns.notStopped = true;
    // boucle infinie
    loop(DatasCom, DatasInterns);

    // destruction des tubes nommés, des sémaphores, ...
    DestructTube("client_to_master");
    DestructTube("master_to_client");

    wait(NULL);

    close(DatasCom.masterToWorker[1]);
    close(DatasCom.workerToMaster[0]);

    MySemDestruct(DatasCom.mutex_1);
    MySemDestruct(DatasCom.mutex_2);
    MySemDestruct(DatasCom.mutex_3);

    return EXIT_SUCCESS;
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien
