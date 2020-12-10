#include "sem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>


#define N_PROC 64	// Maximum number of processors supported
#define DEBUG 1		// 1 -> Print debugging messages
#define N_TESTING 8	// Number of processes we're testing with

// Checks the status of a child process
void checkStatus( int );

// Holds the PIDs for each process
int my_procnum[ N_PROC ];
int main() {

	// mmap a semaphore to be shared among child processes
	struct sem *s = (struct sem *)mmap( NULL, sizeof( struct sem ), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0 );

	// Initialize the semaphore
	// In this test, "producer" processes will add to the total resources
	// "consumer" processes will wait until a resource is available before "using" it
	sem_init( s, 0 );

	// odd i are consumers (calls sem_wait)
	// even i are producers (calls sem_inc)
	for ( int i=0; i<N_TESTING; i++ ) {

		switch ( my_procnum[i] = fork() ) {

		case -1:

			fprintf( stderr, "Error: Failed to fork().\n" );
			perror( "Error" );
			exit( EXIT_FAILURE );

		case 0:

			if ( DEBUG )
				fprintf( stderr, "PID %d from parent PID %d\n", getpid(), getppid() );

			// Each consumer calls sem_wait 1000 times
			// Each producer calls sem_inc 1000 times
			for ( int j=0; j<10000; j++ ) {
				printf("%d\n", j);
				if ( i % 2 ) {
					sem_wait( s );
				}
				else {
					sem_inc( s );
				}

			}

			exit( EXIT_SUCCESS );

		}

	}

	// Check each child process
	for ( int i=0; i<N_TESTING; i++ )
		checkStatus( my_procnum[i] );

}


// Checks the status of a child process
void checkStatus( int pidChild ) {

    int status;

    waitpid( pidChild, &status, 0 );

    // Check if child exited normally
    if( WIFEXITED(status) ) {

        status = WEXITSTATUS(status);
        fprintf( stderr, "Child process (%d) exited with status: %d\n", pidChild, status );

    }
    // Check if child terminated due to unhandled signal
    else if ( WIFSIGNALED(status) ) {

        status = WTERMSIG(status);
        fprintf( stderr, "Child process (%d) terminated due to unhandled signal: %d\n", pidChild, status );

    }
    // Something else caused the child to terminate
    else
        fprintf( stderr, "Child process (%d) terminated due to unknown reason.\n", pidChild );

}

