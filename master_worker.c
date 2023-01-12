#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "myassert.h"

#include "master_worker.h"

/********************************************/
/*          PROJET PROG AVANCEE EN C        */ 
/*     AL NATOUR MAZEN && CAILLAUD TOM      */           
/********************************************/

void create_workers(int number, int fdRead, int fdWrite)
{
	char *argv[5];
    char s_number[128];
    char s_fdRead[128];
    char s_fdWrite[128];
    char s_nextworker[128];
    
    sprintf(s_number, "%d", number);
    sprintf(s_fdRead, "%d", fdRead);
    sprintf(s_fdWrite, "%d", fdWrite);
    sprintf(s_nextworker, "%d", NoNextWorker);

    argv[0] = "worker";
    argv[1] = s_number;
    argv[2] = s_fdRead;
    argv[3] = s_fdWrite;
    argv[4] = s_nextworker;
    argv[5] = NULL;

    execv(argv[0], argv);
    perror("pb avec creation premier fils");
}

