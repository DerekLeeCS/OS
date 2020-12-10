#include "fifo.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

void fifo_init( struct fifo *f ) {

	// Initialize semaphores. Refer to comments in fifo.h for semaphore usage.
	sem_init( &(f->semBufOccupied), 0 );
	sem_init( &(f->semBufEmpty), MYFIFO_BUFSIZ - 1);
	sem_init( &(f->semSpinlockWrite), 1 );
	sem_init( &(f->semSpinlockRead), 1 );

	// The array is intially empty. A read will not be possible until something is written into the zeroth index.
	f->indexRead = 0;
	f->indexWrite = 0;

}

void fifo_wr( struct fifo *f, unsigned long d ) {

	sem_wait( &(f->semBufEmpty) );

	sem_wait( &(f->semSpinlockWrite) );

	f->circularBuffer[ f->indexWrite ] = d;
	f->indexWrite++;

	if( f->indexWrite == MYFIFO_BUFSIZ ) {
		f->indexWrite = 0;
	}

	sem_inc( &(f->semBufOccupied) );

	sem_inc( &(f->semSpinlockWrite) );

}

unsigned long fifo_rd( struct fifo *f ) {

	unsigned long temp;

	sem_wait( &(f->semBufOccupied) );

	sem_wait( &(f->semSpinlockRead) );

	temp = f->circularBuffer[ f->indexRead ];
	f->indexRead++;
	if( f->indexRead == MYFIFO_BUFSIZ ) {
		f->indexRead = 0;
	}

	sem_inc( &(f->semBufEmpty) );

	sem_inc( &(f->semSpinlockRead) );

	return temp;
}
