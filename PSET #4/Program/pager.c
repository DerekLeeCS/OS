#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#define NUM_LINES 23
#define BUF_SIZE 4096

bool checkInput( char* );
bool readDevTTY( int );

int main( int argc, char* argv[] ) {

    if( !freopen(argv[1], "r", stdin) ) {

        fprintf( stderr, "Error: Redirection of stdin to (%s) failed.\n", argv[1]);
        perror( "Error" );
        return -1;

    }

    int terminalFd;
    if( (terminalFd = open("/dev/tty", O_RDWR, 0666)) < 0 ) {

        fprintf( stderr, "Error: Could not open terminal\n" );
        perror( "Error" );
        exit(EXIT_FAILURE);

    }

    bool loop = true;
    size_t size_buf = BUF_SIZE;
    char* str = NULL;

    while ( loop ) {

        for ( int i=0; i<NUM_LINES; i++ ) {

            // Read from STDIN
            getline( &str, &size_buf, stdin );

            // If input is EoF
            if( feof( stdin ) )
                return 0;

            // Print to STDOUT
            fprintf( stdout, "%s", str );

        }

        char message[] = "---Press RETURN for more---\n";
        write( terminalFd, message, strlen(message) );
        loop = readDevTTY( terminalFd );

    }

    if( close(terminalFd) != 0 ) {

        fprintf( stderr, "Error: Cannot close /dev/tty.\n" );
        perror( "Error" );
        exit( EXIT_FAILURE );

    }

    return 0;

}


// Check if input is an exit argument
bool checkInput( char* str ) {

    // EoF is received
    if ( feof( stdin ) )
        return true;

    // Input is q or Q
    if ( strlen( str ) == 2 ) {

        if ( tolower( str[0] ) == 'q' ) {

            fprintf( stderr, "*** Pager terminated by Q command ***\n\n" );
            return true;

        }

    }

    return false;

}


bool readDevTTY(int terminalFd) {

    size_t size_buf = BUF_SIZE;
    char str[BUF_SIZE];

    read( terminalFd, str, size_buf );

    // If input is an exit argument
    if ( checkInput( str ) )
        return false;
    else
        return true;

}
