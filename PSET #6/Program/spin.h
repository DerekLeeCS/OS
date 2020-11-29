#include <sched.h>

struct mutex {

	volatile char lock;
	int count;

};

int spin_lock( struct mutex * );
int spin_unlock( struct mutex * );
