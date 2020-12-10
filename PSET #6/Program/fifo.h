#include "sem.h"

#define MYFIFO_BUFSIZ 4096

struct fifo {

	struct sem semBufOccupied;	// Blocking semaphore, used to keep track of how many words are currently indexed in the FIFO.
					// "s" represents the number of words in the FIFO.
					// Initialize to s = 0. Writers attempt to increase s (up to a maximum of MYFIFO_BUFSIZ), readers attempt to decrease s (down to a minimum of 0).

	struct sem semBufEmpty;		// Blocking semaphore, used to keep track of how many empty indices are in the FIFO (available for writing)
					// "s" represents the number of empty indices in the FIFO.
					// Initialize to s = MYFIFO_BUFSIZ. Readers attempt to increase s (up to a maximum of MYFIFO_BUFSIZ), writers attempt to decrease s (down to a minimum of 0).

	struct sem semSpinlockWrite;	// Spinlock semaphore, used to protect the integrity of the FIFO by allowing only one write process to modify it at a time.
					// "s" represents an available access to the FIFO: s=1 - available; s=0 - unavailable
					// Initialize to s = 1. Accessing the FIFO decrease s to 0; once the access is completed, s is increased to 1 again.

	struct sem semSpinlockRead;	// Spinlock semaphore, used to protect the integrity of the FIFO by allowing only one read process to modify it at a time.
					// "s" represents an available access to the FIFO: s=1 - available; s=0 - unavailable
					// Initialize to s = 1. Accessing the FIFO decrease s to 0; once the access is completed, s is increased to 1 again.

	unsigned long circularBuffer[ MYFIFO_BUFSIZ ];		// This is the FIFO. Although implemented as an array, it will be used as a circular buffer.
	int indexRead;			// The index of the oldest element in the FIFO (i.e. the next element to be read)
	int indexWrite;			// The index of after the newest element in the FIFO (i.e. the next place to be written in/written over).

					// [ ] [ ] [ ] [ ] [ ] [ ] [a] [b] [c] [d] . . .  [x] [y] [z] [ ] [ ] [ ] . . . [ ] [ ] [ ] [ ] [ ] --->Wraps to the start
					//  ^                       ^                                  ^                                 ^
					// circularBuffer[0]       circularBuffer[indexRead]          circularBuffer[indexWrite]        circularBuffer[MYFIFO_BUFSIZ]

};

void fifo_init( struct fifo *f );
void fifo_wr( struct fifo *f, unsigned long d );
unsigned long fifo_rd( struct fifo *f );
