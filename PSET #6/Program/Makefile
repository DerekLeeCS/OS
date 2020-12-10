all: testFifo testSem

testSem:
	gcc -o testSem testSem.c sem.c spin.c tas64.S

testFifo:
	gcc -o testFifo testFifo.c fifo.c sem.c spin.c tas64.S
