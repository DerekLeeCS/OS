#include "tas.h"


struct rwlock {

	// 0, If lock has been acquired
	// 1, Otherwise
	int spinlock;

	// ADD FIELDS TO BE PROTECTED
	int readers;
	int writers;

};


void rw_rdlock( struct rwlock *lock ) {

	for(;;) {

		// While lock has not been acquired
		while( tas( &lock->spinlock ) != 0 )
			;

		if ( lock->writers == 0 )
			break;

		lock->spinlock = 0;
		BLOCK();

	}	// spinlock is held on loop exit

	lock->readers++;
	lock->spinlock = 0;

}


void rw_wrlock( struct rwlock *lock ) {

	for (;;) {

		while ( tas( &lock->spinlock ) != 0 )
			;

		if ( lock->writers == 0 && lock->readers == 0 )
			break;

		lock->spinlock = 0;
		BLOCK();

	}	// spinlock is held on loop exit and readers == 0

	lock->writers++;
	lock->spinlock = 0;

}


void rw_unlock( struct rwlock *lock ) {

	// While lock has not been acquired
	while ( tas( &lock->spinlock ) )
		;

	if ( lock->readers > 0 )
		lock->readers--;

	lock->writers = 0;
	// WAKE UP ANY TASKS WAITING ON lock
	lock->spinlock = 0;

}
