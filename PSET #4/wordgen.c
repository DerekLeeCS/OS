#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define LEN_MIN 3
#define LEN_MAX 15

void randWord( int, char* );

int main( int argc, char* argv[] ) {

    int numWords;

    // Check if proper format of function call
    if ( argc > 2 ) {

        fprintf( stderr, "Error: Too many arguments.\n" );
        exit( EXIT_FAILURE );

    }
    else if ( argc == 2 )
        numWords = atoi( argv[1] );
    else
        numWords = -1;

    int count = 0;
    int wordLen;
    char word[ LEN_MAX ];

    // Seed PRNG
    srand( (unsigned)time(0) );


    while( 1 ) {

        // Break out of loop once limit is reached
        if ( count == numWords )
            break;

        // Get random length
        wordLen = rand() % ( LEN_MAX - LEN_MIN + 1 );
        wordLen += LEN_MIN;

        // Get random word
        randWord( wordLen, word );

        printf( "%s\n", word );
        count++;

    }

    fprintf( stderr, "Finished generating %i candidate words\n", numWords );
    return 0;

}

void randWord( int wordLen, char* word ) {

    char tempChar;
    char tempWord[ LEN_MAX ] = "";

    for ( int i=0; i<wordLen; i++ ) {

        tempChar = 'A' + ( rand() % 26 );
        tempWord[i] = tempChar;

    }

    strcpy( word, tempWord );
    return;

}
