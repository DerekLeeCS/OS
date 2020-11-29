#include "tas.h"
#include "spin.h"

int spin_lock( struct mutex *lp ) {

	// While lock has not been acquired
	while ( tas( &lp->lock ) != 0 )
		sched_yield();

	return 1;

}


int spin_unlock( struct mutex *lp ) {

	lp->lock = 0;

	return 0;

}

