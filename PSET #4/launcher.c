#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>

void performDup( int, int );
void closePipe( int );
int runProcess( int[2], int, char*, char*[], bool );
void checkStatus( int );


int main( int argc, char *argv[] ) {

    char* numWords;

    if ( argc == 2 )
        numWords = argv[1];
    else if ( argc == 1 )
        numWords = (char*)NULL;
    else {

        fprintf( stderr, "Error: Incorrect format for arguments.\n" );
        exit( EXIT_FAILURE );

    }

    // Code based off of stackoverflow.com/questions/21923982

    // char *array[] are used to pass arguments via exec()
    char *cmd1[3]={"wordgen", numWords, NULL};
    char *cmd2[3]={"wordsearch", "words.txt", NULL};
    char *cmd3[2]={"pager", NULL};
    //char *cmd3[2]={"more", NULL};

    // child1 = wordgen, child2 = wordsearch, child3 = pager
    // pipeOne connects child1 - child2 (wordgen, wordsearch)
    // pipeTwo connects child2 - child3 (wordsearch, pager)

    int pidChild[3];
    int pipeOneFds[2], pipeTwoFds[2];

    pidChild[0] = runProcess( pipeOneFds, 0, "./wordgen", cmd1, true );
    pidChild[1] = runProcess( pipeTwoFds, pipeOneFds[0], "./wordsearch", cmd2, true );
    pidChild[2] = runProcess( pipeTwoFds, 2, "./pager", cmd3, false );

    checkStatus( pidChild[2] );
    checkStatus( pidChild[1] );
    checkStatus( pidChild[0] );

    return 0;

}


// Runs a command using a child process
int runProcess( int fdPipe[2], int fdPipeType, char* progName, char* cmd[], bool createPipe ) {

    int pidChild;

    // Make pipe
    if ( createPipe ) {

        if( pipe(fdPipe) == -1 ) {

            fprintf( stderr, "Error: Pipe creation failed.\n" );
            perror( "Error" );
            exit( EXIT_FAILURE );

        }

    }

    int fdPipeDup = -1, stdFileNo;
    bool boolPipe2 = false;

    // Child 1
    if ( fdPipeType == 0 ) {

        fdPipeDup = fdPipe[1];
        stdFileNo = STDOUT_FILENO;

    }
    // Child 3
    else if ( fdPipeType == 2 ) {

        fdPipeDup = fdPipe[0];
        stdFileNo = STDIN_FILENO;

    }
    // Child 2
    else {

        fdPipeDup = fdPipe[1];
        boolPipe2 = true;

    }

    switch( pidChild = fork() ) {

	case -1:

	    fprintf( stderr, "Error: Failed to fork().\n" );
	    perror( "Error" );
	    exit( EXIT_FAILURE );

    // In child
	case 0:

	    // Pipe 1&2
	    if ( boolPipe2 || fdPipeType == 0 )
            	closePipe( fdPipe[0] );

	    // Pipe 1&3
	    if ( !boolPipe2 )
                performDup( fdPipeDup, stdFileNo );
            // Pipe 2
	    else {

            	// Redirect stdin to the read end of pipe one
            	performDup( fdPipeType, STDIN_FILENO );

            	// Redirect stdout to the write end of pipe two
            	performDup( fdPipe[1], STDOUT_FILENO );

	    }

 	    // exec() the new process
	    if( execvp( progName, cmd ) == -1 ) {

            fprintf( stderr, "Error: execvp() failed on program %s.\n", progName );
            perror( "Error" );
            exit( EXIT_FAILURE );

	    }

	    // Just to be safe
	    break;

    // In parent
	default:

        // Pipe 2
        if ( boolPipe2 )
            closePipe( fdPipeType );

        // Pipe 1&2
        if ( fdPipeType != 2 )
            closePipe( fdPipeDup ); // Close the write end of the pipe

        // Pipe 3
        else
            closePipe( fdPipe[0] ); // Close the read end of the pipe


    }

    return pidChild;

}

void performDup( int fd, int fd2 ) {

    if( dup2( fd, fd2 ) == -1 ) {

        fprintf( stderr, "Error: dup2() failed.\n" );
        perror( "Error" );
        exit( EXIT_FAILURE );

    }

}

void closePipe( int fd ) {

    if( close(fd) == -1 ) {

        fprintf( stderr, "Error: Failed to close pipe end.\n" );
        perror( "Error" );
        exit( EXIT_FAILURE );

    }

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
