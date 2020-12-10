#include "spin.h"
#include <signal.h>

#define N_PROC 64

struct sem {

	int semaphore;			// Number of available resources
	int processes[ N_PROC ];	// The ID of processes
	int waitList[ N_PROC ];		// A list of waiting processes
	int curEnd;			// The end of the list
	struct mutex lp;
	sigset_t tempMask;		// Holds the mask that blocks SIGUSR1
	sigset_t emptyMask;		// Holds the mask to reset the signal mask

};


void sem_init( struct sem *, int );
int sem_try( struct sem * );
void sem_wait( struct sem * );
void sem_inc( struct sem * );
