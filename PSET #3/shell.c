#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/time.h>

#define WHITESPACE " \t\n"

int main() {

    char *str = NULL, *token = NULL;
    bool statusExit = false;
    size_t size_buf = 1024;
    int ws = -1; int i=0;

    // Keep track of time
    struct timeval tvalEnd, tvalStart;
    struct rusage usage;

    printf( "Shell execution started.\n" );

    // Stay in shell until exit command is received
    while( !statusExit ) {

        // Read line from STDIN
        getline( &str, &size_buf, stdin );

        // If line begins with #, ignore it
        if ( strncmp( str, "#", 1 ) == 0 )
            continue;

        // Get tokens one by one
        token = strtok( str, WHITESPACE );
        while ( token != NULL ) {

            token = strtok( NULL, WHITESPACE );

            // Get start time
            gettimeofday( &tvalStart, NULL );

            // Child
            if ( fork() == 0 ) {

                printf("In child\n");
                return 0;

            }

            // Get end time
            wait3( &ws, 0, &usage );
            gettimeofday( &tvalEnd, NULL );

            // Measure time
            printf( "Real Time: %f\n",
            ( (double)( tvalEnd.tv_usec - tvalStart.tv_usec ) / 1000000
            + (double)( tvalEnd.tv_sec - tvalStart.tv_sec ) ) );
            printf( "User Time: %f\n",
            ( (double)( usage.ru_utime.tv_usec ) / 1000000
            + (double)( usage.ru_utime.tv_sec ) ) );
            printf( "System Time: %f\n",
            ( (double)( usage.ru_stime.tv_usec ) / 1000000
            + (double)( usage.ru_stime.tv_sec ) ) );

        }

        // Used for testing; breaks after 3 inputs
        i++;
        if ( i == 3 )
            break;

    }

    return 0;

}
