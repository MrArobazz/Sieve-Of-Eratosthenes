#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "myassert.h"

#include "master_worker.h"
#include "Master_Client_Worker.h"



/********************************************/
/*          PROJET PROG AVANCEE EN C        */ 
/*     AL NATOUR MAZEN && CAILLAUD TOM      */           
/********************************************/

/************************************************************************
 * Données persistantes d'un worker
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le worker
// a besoin : le nombre premier dont il a la charge, ...
typedef struct {
    int numworker;              // le nombre du worker
    int worker_next_worker;     // pipe d'un worker vers le prochain worker
    int master_to_worker;       // pipe du master vers le worker
    int worker_to_master;       // pipe du worker vers le master
    int ordre;                  // l'ordre
}workers;

/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <n> <fdIn> <fdToMaster>\n", exeName);
    fprintf(stderr, "   <n> : nombre premier géré par le worker\n");
    fprintf(stderr, "   <fdIn> : canal d'entrée pour tester un nombre\n");
    fprintf(stderr, "   <fdToMaster> : canal de sortie pour indiquer si un nombre est premier ou non\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static void parseArgs(int argc, char * argv[],workers *w)
{
    if (argc != 5)
        usage(argv[0], "Nombre d'arguments incorrect");
    
    // remplir la structure
    int numworker = atoi(argv[1]);
    int master_to_worker = atoi(argv[2]);
    int worker_to_master = atoi(argv[3]);
    int worker_next_worker = atoi(argv[4]);
   
    w->numworker = numworker;
    w->worker_next_worker = worker_next_worker;
    w->master_to_worker = master_to_worker;
    w->worker_to_master = worker_to_master;
}

/************************************************************************
 * Boucle principale de traitement
 ************************************************************************/
// boucle infinie :
    //    attendre l'arrivée d'un nombre à tester
    //    si ordre d'arrêt
    //       si il y a un worker suivant, transmettre l'ordre et attendre sa fin
    //       sortir de la boucle
    //    sinon c'est un nombre à tester, 4 possibilités :
    //           - le nombre est premier
    //           - le nombre n'est pas premier
    //           - s'il y a un worker suivant lui transmettre le nombre
    //           - s'il n'y a pas de worker suivant, le créer
void loop(workers *w)
{
    while(true)
    {
        // on lit l order pour le worker
        w->ordre = ReadData(w->master_to_worker);
        
        //on stock dans des variables les champas de la structure pour + de visbilité
        int ordre = w->ordre;
        int numworker = w->numworker;
        int nextworker = w->worker_next_worker;

        if(ordre == ORDER_STOP)
        {
            if( nextworker !=  NoNextWorker)
            {
                // on envoi l'order de stop au prochain worker
                int ordrestop = ORDER_STOP;
                WriteData(w->worker_next_worker,&ordrestop);
                
                // on attent la mort du worker
                wait(NULL);
                
                // on ferme le tube de worker vers le prochain worker
                CloseTube(w->worker_next_worker);
            }
            break;
        }
        else
        {
            // on lit le nombre a tester le N
            int NumberTest= ReadData(w->master_to_worker);;
        
            if(NumberTest == numworker  )
            {
                // Si le nombre a tester est egale au nombre du worker alors on renvoi success au master
                int success = SUCCESS;
                WriteData(w->worker_to_master,&success);
            }
            else if(NumberTest % numworker == 0)
            {
               // Si le nombre a tester est modulo le nombre du worker alors on renvoi failure au master
                int fail = FAILURE;
                WriteData(w->worker_to_master,&fail);
            }
            else if(nextworker != NoNextWorker)
            {
                // Donc si les 2 premiers tests echoue alors il y a un prochaine worker donc on lui transmet l'order
                WriteData(w->worker_next_worker,&ordre);
                // puis le nombre a tester
                WriteData(w->worker_next_worker,&NumberTest);
            }
            else
            {
                //on cree le tube anonyme
                int tube[2];
                myassert(pipe(tube) != -1,"la creation du tube a echoue");

                //on cree le fils qui va creer les autres workers
                int fils = fork();
                myassert(fils != -1,"la creation du fork a echoue");

                if(fils == 0)
                {
                    //on Ferme le cote du pipe en ecriture
                    int ret_close_fils = close(tube[1]);
                    myassert(ret_close_fils != -1, "la fermeture du tube cote ecriture a echoue");
                    // on cree le prochain worker avec  
                    //              number   , fdRead,    fdWrite
                    create_workers(NumberTest,tube[0],w->worker_to_master);
                }
                //on Ferme le cote du pipe en lecture
                int ret_close = close(tube[0]);
                myassert(ret_close != -1, "la fermeture du tube cote lecture a echoue");
                //on envoie au prochain worker le cote en ecriture
                w->worker_next_worker = tube[1];

                // Donc ici on a cree le prochain worker et donc on lui envoie l'ordre
                WriteData(w->worker_next_worker,&ordre);
                // puis le nombre a tester
                WriteData(w->worker_next_worker,&NumberTest);
            }
        }

    }
}

/************************************************************************
 * Programme principal
 ************************************************************************/

int main(int argc, char * argv[])
{
    workers worke;
    parseArgs(argc, argv,&worke);
    // Si on est créé c'est qu'on est un nombre premier
    // Envoyer au master un message positif pour dire
    // que le nombre testé est bien premier
    loop(&worke);

    // libérer les ressources : fermeture des files descriptors par exemple
    CloseTube(worke.master_to_worker);
    CloseTube(worke.worker_to_master);
 
    return EXIT_SUCCESS;
}
