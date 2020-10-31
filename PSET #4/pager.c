#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define NUM_LINES 23
#define BUF_SIZE 4096

bool checkInput( char* );
bool readDevTTY();

int main( int argc, char* argv[] ) {

    bool loop = true;
    size_t size_buf = BUF_SIZE;
    char* str = NULL;

    while ( loop ) {

        for ( int i=0; i<NUM_LINES; i++ ) {

            // Read from STDIN
            getline( &str, &size_buf, stdin );

            // If input is an exit argument
            if( checkInput( str ) )
                return 0;

            // Print to STDOUT
            printf( "%s\n", str );

        }

        fprintf( stderr, "---Press RETURN for more---\n" );
        loop = readDevTTY();

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

        if ( tolower( str[0] ) == 'q' )
            return true;

    }

    return false;

}


bool readDevTTY() {

    size_t size_buf = BUF_SIZE;
    char* str = NULL;

    FILE *fp = fopen( "/dev/tty", "r" );
    if ( fp == NULL ) {

        fprintf( stderr, "Error: Cannot open /dev/tty.\n" );
        perror( "Error" );
        exit( EXIT_FAILURE );

    }

    getline( &str, &size_buf, fp );

    // Close the file
    if( fclose(fp) != 0 ) {

        fprintf( stderr, "Error: Cannot close /dev/tty.\n" );
        perror( "Error" );
        exit( EXIT_FAILURE );

    }

    // If input is an exit argument
    if ( checkInput( str ) )
        return false;
    else
        return true;

}
