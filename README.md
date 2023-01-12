# Sieve Of Eratosthenes

## The project

The project is completely written in French.
It was a semester end exam project based on everything we have seen : the advanced C, the processus and communication between them.
We manipulated big processus : a client, a master and several workers who make big calculs to write the Sieve of Eratosthenes in pipe-line version, an Hoare's algorithm.
We were two students to code that project.

The project is completely done.

## To compilate

- "make" (or "make all") : compile the entire project
- "make <exe_name>" : compile one module
     so 3 possibilities : "make master", "make worker" or "make client"
- "make <fichier.o>" : compile one .c file
     for example "make master_client.o" to compile master_client.c
- "make clean" : delete .o and .d files
- "make distclean" : delete .o, .d and .exe files
- you need to adapt the Makefile if you add source files

Remove named pipes and semaphores :
- use rmsempipe.sh
- it removes all semaphores with 641 rights
- it removes the both named pipes with hard-written names
- you need to adapt the script if necessary

Others :
- a module "myassert" that works like "assert" in C but with more precise messages
- cf. myassert.h for doc
