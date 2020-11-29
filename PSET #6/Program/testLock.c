#include "spin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>


#define N_PROC 64	// Maximum number of processors supported
#define LOCK 1		// 1 -> Use spin lock
#define DEBUG 0		// 1 -> Print debugging messages

// Checks the status of a child process
void checkStatus( int );

// Holds the PIDs for each process
int my_procnum[ N_PROC ];


int main() {

	char *addr;
	size_t size_buf = 2048;

	struct mutex *lp = (struct mutex *)mmap( NULL, size_buf, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0 );

	// Initialize the struct
	lp->count = 0;
	lp->lock = 0;

	for ( int i=0; i<3; i++ ) {

		switch ( my_procnum[i] = fork() ) {

		case -1:

			fprintf( stderr, "Error: Failed to fork().\n" );
			perror( "Error" );
			exit( EXIT_FAILURE );

		case 0:

			if ( DEBUG )
				fprintf( stderr, "PID %d from parent PID %d\n", getpid(), getppid() );

			for ( int j=0; j<100000; j++ ) {

				// Use a spin lock
				if ( LOCK ) {

					if ( spin_lock( lp ) ) {

						lp->count++;
						spin_unlock( lp );

					}

				} else {

					lp->count++;

				}

			}

			if ( DEBUG )
				fprintf( stderr, "Child Count: %d\n", lp->count );

			exit( EXIT_SUCCESS );

		}

	}

	// Check each child process
	for ( int i=0; i<3; i++ )
		checkStatus( my_procnum[i] );

	fprintf( stderr, "Final Count: %d\n", lp->count );

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

