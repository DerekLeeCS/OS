#include "sem.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define DEBUG 0			// 1 -> Enable debugging functionality

void sigusr1_handler( int signum ) {}

void sem_init( struct sem *s, int count ) {

	// Store the number of resources available
	s->semaphore = count;

	// Stores the current end of the list
	s->curEnd = 0;

	// Initialize the arrays
	for ( int i=0; i<N_PROC; i++ ) {

		s->processes[i] = 0;
		s->waitList[i] = 0;

	}

	// Initialize the mutex lock
	(s->lp).lock = 0;
	(s->lp).count = 0;

	// Initialize the empty mask
	if ( sigemptyset( &s->emptyMask ) != 0 ) {

		fprintf( stderr, "Error: Cannot initialize emptyMask.\n" );
		perror( "Error" );
		exit( EXIT_FAILURE );

	}

	// Initialize the temp mask with SIGUSR1
	if ( sigemptyset( &s->tempMask ) != 0 ) {

		fprintf( stderr, "Error: Cannot initialize tempMask.\n" );
		perror( "Error" );
		exit( EXIT_FAILURE );

	}
	if ( sigaddset( &s->tempMask, SIGUSR1 ) != 0 ) {

		fprintf( stderr, "Error: Cannot add SIGUSR1 to tempMask.\n" );
		perror( "Error" );
		exit( EXIT_FAILURE );

	}

	struct sigaction new_action;
	new_action.sa_handler = sigusr1_handler;
	new_action.sa_flags = 0;
	if ( sigaction( SIGUSR1, &new_action , NULL ) != 0 ) {

		fprintf( stderr, "Error: Cannot use sigaction to change the disposition of SIGUSR1.\n" );
		perror( "Error" );
		exit( EXIT_FAILURE );

	}

}


int sem_try( struct sem *s ) {

	// Decrement the semaphore by 1
	if ( spin_lock( &(s->lp) ) ) {

		// If semaphore is not positive, returns immediately
		if ( s->semaphore < 1 ) {

			return 0;

		}

		s->semaphore--;
		spin_unlock( &(s->lp) );
		return 1;

	}

	// SHOULD NEVER HAPPEN
	return -1;

}


void sem_wait( struct sem *s ) {

	int retVal;

	// Attempt to decrement and block until successful
	while(1) {

		retVal = sem_try(s);

		// If successful, exit the loop
		if ( retVal > 0 )
			break;

		// If sem_try returned 0 (blocking), then spinlock is already engaged here!

		// Block wakeup signal
		if ( sigprocmask( SIG_BLOCK, &s->tempMask, NULL ) != 0 ) {
			fprintf( stderr, "Error: Cannot block SIGUSR1.\n" );
			perror( "Error" );
			exit( EXIT_FAILURE );
		}

		s->waitList[ s->curEnd ] = getpid();
		s->curEnd++;
		spin_unlock( &(s->lp) );


		// Reset the blocked signal and sleep
		sigsuspend( &s->emptyMask );

	}

}


void sem_inc( struct sem *s ) {

	if ( spin_lock( &(s->lp) ) ) {

		s->semaphore++;

		if ( s->curEnd > 0 ) {

			// Send wakeup signal to all sleepers
			for ( int i=0; i< s->curEnd; i++ ) {

				kill( s->waitList[ i ], SIGUSR1 );
				// Use spin_lock() to protect waitlist

			}

			s->curEnd = 0;
		}

		spin_unlock( &(s->lp) );

	}

}
