#include "fifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>

#define N_PROC 64	// Maximum number of processors supported
#define DEBUG 1		// 1 -> Print debugging messages
#define N_READER 1	// Number of processes that read from the FIFO. Must be at least one.
			// FOR NOW, LEAVE N_READER AT 1
			// We're testing one reader, multiple writers. We'll test multiple readers later.

#define N_WRITER 8	// Number of processes that write to the FIFO. Must be at least one.
#define N_WRCOUNT 10000	// Number of times each writer writes to the FIFO
			// Keep this less than INT_MAX, okay?

int my_procnum[ N_PROC ];

// Checks the status of a child process
void checkStatus( int );

// Holds the PIDs for each process
//int my_procnum[ N_PROC ];

// Holds the value of the latest datum recieved from each writer
// Indexed by writer number
// e.g. assume N_WRITER = 3.
// If latestDatum is: [32] [84] [29]
// - Writer 0's latest datum is 32
// - Writer 1's latest datum is 84
// - Writer 2's latest datum is 29
// Note: even though the FIFO accepts unsigned long's, we only use the first ten DECIMAL places to send the data (This should be enough for data up to the size of INT_MAX). The rest of the datum keeps track of the writer number.
// e.g. assume we recieve the datum: 1'2'147'483'647 ( ' added for clarity )
// 	The 2'147'483'647 is the value of the data
//	Any digits more significant than the ten least significant digits are the writer number (here, it's writer number 1)
int latestData[ N_WRITER ];

int main() {

	// mmap a FIFO to be shared among child processes
	struct fifo *f = (struct fifo *)mmap( NULL, sizeof( struct fifo ), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0 );

	// Initialize the FIFO
	fifo_init( f );
	// The first N_READER processes will be readers, the next N_WRITER processes will be writers.
	for ( int i=0; i<( N_READER + N_WRITER ); i++ ) {

		switch ( my_procnum[i] = fork() ) {

		case -1:

			fprintf( stderr, "Error: Failed to fork().\n" );
			perror( "Error" );
			exit( EXIT_FAILURE );

		case 0:

			if ( DEBUG )
				fprintf( stderr, "PID %d from parent PID %d\n", getpid(), getppid() );

			if ( i < N_READER ) {
			// This is a reader process

				for( int j = 0; j< (N_WRITER * N_WRCOUNT); j++ ) {
					unsigned long temp = fifo_rd( f );
					unsigned long dataNumber = temp % 10000000000;
					unsigned long writerNumber = (temp - (temp % 10000000000)) / 10000000000;
					if( (int)dataNumber == latestData[ (int)writerNumber ] ) {
						latestData[ (int)writerNumber ]++;
					}
					else {
						fprintf( stderr, "Error: reader expected data [%d] from writer number [%d], instead recieved data [%d]\n", latestData[ (int)writerNumber ], (int)writerNumber, (int)dataNumber );
					}
				}

				for ( int i=0; i<N_WRITER; i++ ) {
					printf( "Successfully read sequential data up to [%d] from writer [%d]\n", latestData[i], i );
				}


			}
			else {
			// This is a writer process

				for( int j = 0; j<N_WRCOUNT; j++ ) {

					int writerNumber = i - N_READER;
					unsigned long datum = writerNumber*10000000000 + j;
					fifo_wr( f, datum );
				}

			}

			printf( "Process %d done\n", getpid() );
			exit( EXIT_SUCCESS );

		}

	}

	// Check each child process
	for ( int i=0; i< ( N_READER + N_WRITER) ; i++ )
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

