#ifndef MASTER_CLIENT_WORKER
#define MASTER_CLIENT_WORKER


/********************************************/
/*          PROJET PROG AVANCEE EN C        */ 
/*     AL NATOUR MAZEN && CAILLAUD TOM      */           
/********************************************/

void CreateTube(const char * file, int flags);				// operation de creation d'un tube nommé
void DestructTube(const char * file);						// operation de destruction d'un tube nommé
int OpenTube(const char * file, int flags);					// operation d'ouverture d'un tube nommé avec un certains flags (ecriture,lecture,etc...)
void CloseTube(int thetube);								// operation de fermeture d'un tube nommé
void WriteData(int thetube, int *data);						// operation d'ecriture d'une data sur un tube nommé
int ReadData(int thetube);									// operation de lecture d'une data sur un tube nommé


#endif