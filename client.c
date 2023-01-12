#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include "myassert.h"

#include "master_client.h"
#include "Master_Client_Worker.h"

// chaines possibles pour le premier paramètre de la ligne de commande
#define TK_STOP      "stop"
#define TK_COMPUTE   "compute"
#define TK_HOW_MANY  "howmany"
#define TK_HIGHEST   "highest"
#define TK_LOCAL     "local"


/********************************************/
/*          PROJET PROG AVANCEE EN C        */ 
/*     AL NATOUR MAZEN && CAILLAUD TOM      */           
/********************************************/

/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <ordre> [<nombre>]\n", exeName);
    fprintf(stderr, "   ordre \"" TK_STOP  "\" : arrêt master\n");
    fprintf(stderr, "   ordre \"" TK_COMPUTE  "\" : calcul de nombre premier\n");
    fprintf(stderr, "                       <nombre> doit être fourni\n");
    fprintf(stderr, "   ordre \"" TK_HOW_MANY "\" : combien de nombres premiers calculés\n");
    fprintf(stderr, "   ordre \"" TK_HIGHEST "\" : quel est le plus grand nombre premier calculé\n");
    fprintf(stderr, "   ordre \"" TK_LOCAL  "\" : calcul de nombres premiers en local\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static int parseArgs(int argc, char * argv[], int *number)
{
    int order = ORDER_NONE;

    if ((argc != 2) && (argc != 3))
        usage(argv[0], "Nombre d'arguments incorrect");

    if (strcmp(argv[1], TK_STOP) == 0)
        order = ORDER_STOP;
    else if (strcmp(argv[1], TK_COMPUTE) == 0)
        order = ORDER_COMPUTE_PRIME;
    else if (strcmp(argv[1], TK_HOW_MANY) == 0)
        order = ORDER_HOW_MANY_PRIME;
    else if (strcmp(argv[1], TK_HIGHEST) == 0)
        order = ORDER_HIGHEST_PRIME;
    else if (strcmp(argv[1], TK_LOCAL) == 0)
        order = ORDER_COMPUTE_PRIME_LOCAL;
    
    if (order == ORDER_NONE)
        usage(argv[0], "ordre incorrect");
    if ((order == ORDER_STOP) && (argc != 2))
        usage(argv[0], TK_STOP" : il ne faut pas de second argument");
    if ((order == ORDER_COMPUTE_PRIME) && (argc != 3))
        usage(argv[0], TK_COMPUTE " : il faut le second argument");
    if ((order == ORDER_HOW_MANY_PRIME) && (argc != 2))
        usage(argv[0], TK_HOW_MANY" : il ne faut pas de second argument");
    if ((order == ORDER_HIGHEST_PRIME) && (argc != 2))
        usage(argv[0], TK_HIGHEST " : il ne faut pas de second argument");
    if ((order == ORDER_COMPUTE_PRIME_LOCAL) && (argc != 3))
        usage(argv[0], TK_LOCAL " : il faut le second argument");
    if ((order == ORDER_COMPUTE_PRIME) || (order == ORDER_COMPUTE_PRIME_LOCAL))
    {
        *number = strtol(argv[2], NULL, 10);
        if (*number < 2)
             usage(argv[0], "le nombre doit être >= 2");
    }       
    
    return order;
}
/************************************************************************
 * Fonction Affichage Reponse
 ************************************************************************/
void PrintReponse(int order, int reponse, int number)
{
    switch (order) {
        case ORDER_COMPUTE_PRIME :
            {
                if(reponse == 1)
                printf("\n>> Le nombre %d est un nombre premier.\n\n",number );
                else
                printf("\n>> Le nombre %d n'est pas un nombre premier.\n\n",number );
            }
            break;
        case ORDER_HOW_MANY_PRIME :
            printf("\n>> Il y a eu %d nombres premiers envoyés par les clients.\n\n",reponse);
            break;
        case ORDER_HIGHEST_PRIME :
            printf("\n>> Le nombre premier le plus grand actuellement est %d.\n\n",reponse);
            break;
        case ORDER_STOP :
            {
                if(reponse == ORDER_STOP)
                    printf("\n>> Le Master s'est arreter correctement.\n\n");
                else
                    printf("\n>> Le Master ne s'est pas arreter correctement.\n\n");
            }
            break;
        case ORDER_NONE :
            printf("\n>> C'est ORDER_NONE, ce n'est pas normal.\n\n");
            break;
        default :
            printf("\n>> Cette order n'existe pas ! \n\n");
            break;
    }
}
/************************************************************************
 * Fonction Aux Pour les threads
 ************************************************************************/

//====================================================
// structure pour passer les paramètres aux pthread :
typedef struct
{
    bool *tab;               // le tableau de booléens
    int tabsize;             // la taille du tableau
    int val;                 // la valeur du thread courant
    pthread_mutex_t *Mutex;  // pour protéger les accès aux tableaux
} ThreadData;
//====================================================
// Fonction support d'un thread
// Tous les threads lanceront cette fonction
void * codeThread(void * arg)
{
    ThreadData *ptr = (ThreadData*) arg;
    pthread_mutex_lock(ptr->Mutex);
    int valthread = ptr->val;
    int tailletab = ptr->tabsize;
    for (int i = 2 ; i*valthread < tailletab ; ++i)
    {
        ptr->tab[i*valthread] = false;
    }
    pthread_mutex_unlock(ptr->Mutex);
    return NULL;
}
//====================================================
//Fonction de creation et d'initialisation d'une structure threaddata
ThreadData* InitCreatThreadData(pthread_mutex_t *m, bool *tab, int number, int nbthreads)
{
    ThreadData * data = (ThreadData *)malloc(sizeof(ThreadData)*nbthreads);
    for (int i = 0 ; i < nbthreads; ++i)
    {
        data[i].tab = tab;
        data[i].Mutex = m;
        data[i].tabsize = number;
        data[i].val = i;
    }
    return data;
}
//Fonction de creation et d'initialisation d'un tableau de booleen
bool* InitCreatTabBool(int number)
{
    bool *res = malloc(sizeof(bool)*number);
    for (int j = 0; j < number; ++j)
    {
        res[j] = true;
    }
    res[0] = false;
    res[1] = false;

    return res;
}
//Fonction d'affichage du tableau de booleen
void PrintTabBool(const bool *tab, int number)
{   
    printf(">> Les nombres premiers jusqu a %d sont :\n",number );
    for (int i = 0; i < number; ++i)
    {
        if(tab[i])
            printf("* %d\n",i );
    }
}

void LocalThreadCompute(int number)
{
    int nbthreads = (int) (sqrt(number)-1);
    nbthreads += 2; // je suis obligé de rajouter 2 car on part toujours de la 2eme case pour la creation des thread ou la modification du tableau de booleen ainsi 
                    // les 2 premiers thread ne seront jamais utilises et donc les cases du tableau de booleens ne seront jamais bien initialiser cela ne change rien 
                    // concernant l enonce, hormis d'avoir 2 cases dans le tableau de threads inutiles mais le nombre de thread utilise est bien sqrt(number)-1

    pthread_t tabthreads[nbthreads];
    pthread_mutex_t monMutex = PTHREAD_MUTEX_INITIALIZER;

    bool *tabBool = InitCreatTabBool(number);
    ThreadData *data = InitCreatThreadData(&monMutex,tabBool, number,nbthreads);

    for (int i = 2 ; i < nbthreads; i++)
    {

        int creat_ret = pthread_create(&(tabthreads[i]), NULL, codeThread, &data[i]);
        myassert(creat_ret == 0, "La creation d un thread a echoue");
    }

    for (int i = 2; i < nbthreads; i++)
    {
        int join_ret = pthread_join(tabthreads[i], NULL);
        myassert(join_ret == 0, "L attente d un thread a echoue");
    }
       
    PrintTabBool(tabBool,number);
   
    //Liberation des ressources 
    pthread_mutex_destroy(&monMutex);
    free(tabBool);
    free(data);
}

/************************************************************************
 * Fonction principale
 ************************************************************************/
int main(int argc, char * argv[])
{
    int number = 0;
    int order = parseArgs(argc, argv, &number);

    if (order == ORDER_COMPUTE_PRIME_LOCAL)
    {
        LocalThreadCompute(number);
    }
    else {
        // entrer SC entre clients
        int semIdClient = MySemGet(MUTEX1_ID);
        PrendreSem(semIdClient);

        sleep(1); // Ce sleep permet au client d'etre sur d'ecrire sur un tube ouvert des 2 cote (CF: rapport.pdf) Vu avec M.Subrenat Gilles
      
        //ouverture tubes
        int client_to_master = OpenTube("client_to_master", O_WRONLY);
        int master_to_client = OpenTube("master_to_client",O_RDONLY);
    
        //envoie donnees
        WriteData(client_to_master,&order);

        if (order == ORDER_COMPUTE_PRIME)
        {
            //ecriture du number au master
             WriteData(client_to_master,&number);         
        }
            //lecture de la reponse du master
            int ReponseMaster_ret = ReadData(master_to_client);;

            // affichage resultat
            PrintReponse(order,ReponseMaster_ret,number);

            //liberation ressources

            int semIdClientMaster = MySemGet(MUTEX2_ID);
            int semIdClientMaster2 = MySemGet(MUTEX3_ID);

            //debut synchro type rendez-vous
            VendreSem(semIdClientMaster);
            CloseTube(client_to_master);
            CloseTube(master_to_client);
            // sortie SC entre clients
            VendreSem(semIdClient);
            //fin synchro type rendez-vous
            VendreSem(semIdClientMaster2); 
    }
    return EXIT_SUCCESS;
}


 // order peut valoir 5 valeurs (cf. master_client.h) :
    //      - ORDER_COMPUTE_PRIME_LOCAL
    //      - ORDER_STOP
    //      - ORDER_COMPUTE_PRIME
    //      - ORDER_HOW_MANY_PRIME
    //      - ORDER_HIGHEST_PRIME
    //
    // si c'est ORDER_COMPUTE_PRIME_LOCAL
    //    alors c'est un code complètement à part multi-thread
    // sinon
    //    - entrer en section critique :
    //           . pour empêcher que 2 clients communiquent simultanément
    //           . le mutex est déjà créé par le master
    //    - ouvrir les tubes nommés (ils sont déjà créés par le master)
    //           . les ouvertures sont bloquantes, il faut s'assurer que
    //             le master ouvre les tubes dans le même ordre
    //    - envoyer l'ordre et les données éventuelles au master
    //    - attendre la réponse sur le second tube
    //    - sortir de la section critique
    //    - libérer les ressources (fermeture des tubes, ...)
    //    - débloquer le master grâce à un second sémaphore (cf. ci-dessous)
    // 
    // Une fois que le master a envoyé la réponse au client, il se bloque
    // sur un sémaphore ; le dernier point permet donc au master de continuer
    //
    // N'hésitez pas à faire des fonctions annexes ; si la fonction main
    // ne dépassait pas une trentaine de lignes, ce serait bien.
