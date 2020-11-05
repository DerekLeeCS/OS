#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>

#define BUF_SIZE 4096
#define MAX_WORDS 1000000
#define LEN_MAX 15

void loadDictionary( char[][LEN_MAX+1], FILE* );
void checkDictionary( char[][LEN_MAX+1] );

int curPos = 0;
jmp_buf wormhole;

void handler( int sig ) {

    int id = getpid();
    fprintf( stderr, "PID %d received signal %d.\n", id, sig );
    longjmp( wormhole, id );

}

int main( int argc, char* argv[] ) {

    // Use LEN_MAX+1 to account for the \n character
    char (*dictionary)[LEN_MAX+1] = malloc( sizeof( char[MAX_WORDS][LEN_MAX+1] ) );

    bool loop = true;
    FILE* fp;

    if ( argc == 2 ) {

        fp = fopen( argv[1], "r" );
        if ( fp == NULL ) {

            fprintf( stderr, "Error: Cannot open file (%s) for reading.\n", argv[1] );
            perror( "Error" );
            exit( EXIT_FAILURE );

        }

    }
    else {

        fprintf( stderr, "Error: Wrong format for arguments.\n" );
        exit( EXIT_FAILURE );

    }

    loadDictionary( dictionary, fp );
    checkDictionary( dictionary );

    return 0;

}

void loadDictionary( char dictionary[][LEN_MAX+1], FILE* fp ) {

    size_t size_buf = BUF_SIZE;
    char *str = NULL, *c, *it;

    // Read into dictionary
    while ( getline( &str, &size_buf, fp ) != -1 ) {

        // Convert to uppercase
        for ( it=str; *it != '\0'; it++ )
            *it = toupper( *it );

        // Add the word only if it's not too long
        if( strlen(str) <= LEN_MAX+1 ) {

            strcpy( dictionary[ curPos ], str );
            curPos++;

        }

    }

    fprintf( stderr, "Accepted %i words\n", curPos );
    return;

}


// Reads input from STDIN and checks each line against dictionary
void checkDictionary( char dictionary[][LEN_MAX+1] ) {

    size_t size_buf = BUF_SIZE;
    char* str = NULL;
    int numMatch = 0;

    signal( SIGPIPE, handler );

    if ( !setjmp( wormhole ) ) {

        while ( getline( &str, &size_buf, stdin ) != -1 ) {

            // Convert to uppercase
            for ( char *it=str; *it != '\0'; it++ )
                *it = toupper( *it );

            for ( int i=0; i<curPos; i++ ) {

                if ( strcmp( str, dictionary[i] ) == 0 ) {

                    printf( "%s", str );
                    numMatch++;
                    break;

                }

            }

        }

    }

    fprintf( stderr, "Matched %i words\n", numMatch );
    return;

}
