#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <fcntl.h>

#define WHITESPACE " \t\n"
#define PATH_MAX 1024
#define ARG_MAX 1024

// Probably not needed
// token = NULL line

// Handles default commands
bool shellCommands( char * );

// Parses I/O redirection
int ioRedirect( char * );

// For exit command, also for EOF
bool statusExit = false;
int lastExitStatus = 0;

int main() {

    char *str = NULL, *token = NULL;
    size_t size_buf = 1024;
    int ws = -1;

    // Keep track of time
    struct timeval tvalEnd, tvalStart;
    struct rusage usage;

    printf( "Shell execution started.\n\n" );

    // Stay in shell until exit command is received
    while( !statusExit ) {

        // Read line from STDIN
        getline( &str, &size_buf, stdin );

        // Reached EoF
        if ( feof( stdin ) )
            break;

        // If line begins with #, ignore it
        if ( strncmp( str, "#", 1 ) == 0 )
            continue;

        printf("\nProcessing input: %s", str);

        // Get the first token
        token = strtok( str, WHITESPACE );
        printf("\tCurrent Token: %s\n", token);

        // Check if token is a shell command
        if ( shellCommands( token ) )
            continue;

        // Get start time
        gettimeofday( &tvalStart, NULL );

        // Child
        if ( fork() == 0 ) {

            printf( "In child\n" );

            bool shouldExec = true;
            int argCount = 0;
            char argVector[ARG_MAX][PATH_MAX];

            // Gets tokens one by one
            while ( token != NULL ) {

                // First see if the token is a redirection
                int retVal = ioRedirect(token);

                // If any redirection fails, exit with an error status
                if( retVal == -1 ) {

                    fprintf(stderr, "Error redirecting I/O, exec() cancelled.\n");
                    shouldExec = false;
                    lastExitStatus = -1;
                    break;

                }
                // If ioRedirect() returns 1, it didn't fail, but the token was not a redirect instruction
                else if( retVal == 1 ) {

                    strcpy( argVector[argCount], token );
                    argCount++;
                    printf( "\t\tArgument recorded in argVector[%d]: %s\n", argCount-1, token ); // REMOVE THIS LINE AFTER YOU FINISH DEBUGGING, OTHERWISE IT'LL GET PRINTED TO THE STDOUT YOU REDIRECT TO

                }

                // Get next token
                token = strtok( NULL, WHITESPACE );
                printf( "\tCurrent Token: %s\n", token ); // REMOVE THIS LINE AFTER YOU FINISH DEBUGGING, OTHERWISE IT'LL GET PRINTED TO THE STDOUT YOU REDIRECT TO

            } // closes while (token != NULL)

            // TEST LOOP // REMOVE AFTER DEBUG
            printf("\nFor Debugging - Contents of argVector are:\n");
            for( int j=0; j<argCount; j++ ) {

                // If you redirected stdout, this will print to there!
                printf( "argVector[%d]: %s\n", j, argVector[j] );

            }

            // argv should be NULL terminated (last element is NULL)
            if( shouldExec ) {

                /* exec() stuff here */
                // The command will be in argVector[0]
                int err = execvp( argVector[0], argVector );

                // Exec returns -1 on error
                if ( err == -1 ) {

                    fprintf( stderr, "Exec for command %s failed.\n", argVector[0] );
                    perror( "Error" );
                    exit( 127 );

                }

            }

            return 0;

        }

        // Get end time and exit status
        wait3( &ws, 0, &usage );
        gettimeofday( &tvalEnd, NULL );
        lastExitStatus = WEXITSTATUS( ws );

        // Measure time
        printf( "\nReal Time: %f\n",
            ( (double)( tvalEnd.tv_usec - tvalStart.tv_usec ) / 1000000
            + (double)( tvalEnd.tv_sec - tvalStart.tv_sec ) ) );
        printf( "User Time: %f\n",
            ( (double)( usage.ru_utime.tv_usec ) / 1000000
            + (double)( usage.ru_utime.tv_sec ) ) );
        printf( "System Time: %f\n\n",
            ( (double)( usage.ru_stime.tv_usec ) / 1000000
            + (double)( usage.ru_stime.tv_sec ) ) );

    }// closes while( !statusExit )

    printf( "\nShell exiting with a status of: %d\n", lastExitStatus ); // For testing purposes
    return lastExitStatus;

}// closes main()


// Handles default commands
// Returns true if token is a default command
// Returns false otherwise
bool shellCommands( char *token ) {

    // Handles empty line
    if ( token == NULL )
        return true;

    // Handle built in command: cd {dir}
    else if( strcmp( token, "cd" ) == 0 ) {

        // Gets next token
        token = strtok( NULL, WHITESPACE );
        printf( "Changing directory to: %s \n\n", token );

        // If directory is specified, change to it
        if( token != NULL ) {

            if( chdir(token) == -1 ) {

                fprintf( stderr, "Could not cd to: %s\n", token );
                perror( "Error" );
                lastExitStatus = -1;

            }

        }
        // Otherwise, changes to home directory
        else {

            if( chdir(getenv("HOME")) == -1 ) {

                fprintf(stderr, "Could not cd to home directory. Home: %s\n", getenv("HOME"));
                perror( "Error" );
                lastExitStatus = -1;

            }

        }

        return true;

    }

    // Handle built in command: pwd
    else if( strcmp( token, "pwd" ) == 0 ) {

        char currentDir[PATH_MAX];
        if( getcwd( currentDir, sizeof(currentDir) ) )
            printf( "Current working directory: %s\n\n", currentDir );
        else {

            fprintf( stderr, "Could not getcwd()\n" );
            perror( "Error" );
            lastExitStatus = 1;

        }

        return true;

    }

    // Handle built in command: exit {status}
    else if( strcmp( token, "exit" ) == 0 ) {

        statusExit = true;

        // Attempt to read the optional {status}. If it is given, set the last exit status to it.
        if( token=strtok( NULL, WHITESPACE ) )
            lastExitStatus = atoi( token );

        return true;

    }

    return false;

}




// Returns -1 if an error was encountered
// Returns 0 if redirection was successful
// Returns 1 if token is not a redirection
int ioRedirect( char *token ) {

	char filePath[PATH_MAX];
	strcpy( filePath, token );
	int fd = -1;

    // <filename: redirect stdin
	if( strncmp( token, "<", 1 ) == 0 ) {

		memmove( filePath, filePath+1, strlen(filePath) );
		printf("\t\tioRedirect: \'<\' detected, File Path: %s\n", filePath);

		if ( ( fd=open(filePath, O_RDONLY, 0666) ) < 0 ) {

			fprintf(stderr, "Couldn't open file (%s) while redirecting STDIN\n", filePath);
			perror( "Error" );
			return -1;

		}
		if ( dup2(fd,0) < 0 ) {

            fprintf(stderr, "Couldn't dup2 fd of file (%s) to STDIN\n");
			perror( "Error" );
			return -1;

		}
		close(fd);

		return 0;

	}

    // >>filename: redirect stdout (APPEND)
	if( strncmp( token, ">>", 2 ) == 0 ) {

		memmove( filePath, filePath+2, strlen(filePath) );
		printf( "\t\tioRedirect: \'>>\' detected, File Path: %s\n", filePath );

		if ( ( fd=open(filePath, O_WRONLY|O_CREAT|O_APPEND, 0666) ) < 0 ) {

            fprintf(stderr, "Couldn't open file (%s) while redirecting STDOUT (APPEND)\n", filePath);
            perror( "Error" );
			return -1;
		}
		if ( dup2(fd,1) < 0 ) {

			fprintf(stderr, "Couldn't dup2 fd of file (%s) to STDOUT (APPEND)\n");
			perror( "Error" );
			return -1;

		}
		close(fd);

		return 0;

	}

    // >filename: redirect stdout (TRUNC)
	if( strncmp( token, ">", 1 ) == 0 ) {

		memmove( filePath, filePath+1, strlen(filePath) );
		printf( "\t\tioRedirect: \'>\' detected, File Path: %s\n", filePath );

		if ( ( fd=open(filePath, O_WRONLY|O_CREAT|O_TRUNC, 0666) ) < 0 ) {

			fprintf(stderr, "Couldn't open file (%s) while redirecting STDOUT (TRUNC)\n", filePath);
			perror( "Error" );
			return -1;

		}
		if ( dup2(fd,1) < 0 ) {

			fprintf(stderr, "Couldn't dup2 fd of file (%s) to STDOUT (TRUNC)\n");
			perror( "Error" );
			return -1;

		}
		close(fd);

		return 0;

	}

    // 2>>filename: redirect stderr (APPEND)
	if( strncmp( token, "2>>", 3 ) == 0 ) {

		memmove( filePath, filePath+3, strlen(filePath) );
		printf( "\t\tioRedirect: \'2>>\' detected, File Path: %s\n", filePath );

		if ( ( fd=open(filePath, O_WRONLY|O_CREAT|O_APPEND, 0666) ) < 0 ) {

			fprintf(stderr, "Couldn't open file (%s) while redirecting STDERR (APPEND)\n", filePath);
			perror( "Error" );
			return -1;

		}
		if ( dup2(fd,2) < 0 ) {

			fprintf(stderr, "Couldn't dup2 fd of file (%s) to STDERR (APPEND)\n");
			perror( "Error" );
			return -1;

		}
		close(fd);

		return 0;

	}


    // 2>filename: redirect stdout (TRUNC)
	if( strncmp( token, "2>", 2 ) == 0 ) {

		memmove( filePath, filePath+2, strlen(filePath) );
		printf("\t\tioRedirect: \'2>\' detected, File Path: %s\n", filePath);

		if ( ( fd=open(filePath, O_WRONLY|O_CREAT|O_TRUNC, 0666) ) < 0 ) {

			fprintf(stderr, "Couldn't open file (%s) while redirecting STDERR (TRUNC)\n", filePath);
			perror( "Error" );
			return -1;
		}
		if ( dup2(fd,2) < 0 ) {

			fprintf(stderr, "Couldn't dup2 fd of file (%s) to STDERR (TRUNC)\n");
			perror( "Error" );
			return -1;

		}
		close(fd);

		return 0;

	}

	// Return value of 1 indicates no error and no redirection (the token is an argument)
	return 1;

}
