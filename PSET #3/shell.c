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

// Handles default commands
bool shellCommands( char * );

// Parses I/O redirection
int ioRedirect( char * );

// For exit command, also for EOF
bool statusExit = false;
int lastExitStatus = 0;

int main( int argc, char *argv[] ) {

    char *str = NULL, *token = NULL;
    size_t size_buf = 1024;
    int ws = -1;

    // Keep track of time
    struct timeval tvalEnd, tvalStart;
    struct rusage usage;

    FILE *fp;

    // If given a file, try to open it
    if ( argv[1] != NULL ) {

        fp = fopen( argv[1], "r" );
        if ( fp == NULL ) {

          fprintf( stderr, "Cannot open input file: %s\n", argv[1] );
          perror( "Error" );
          lastExitStatus = -1;
          statusExit = true;

        }

    }
    else
        fp = stdin;

    fprintf( stderr, "Shell execution started.\n\n" );

    // Stay in shell until exit command is received
    while( !statusExit ) {

        // Read line from fd
        getline( &str, &size_buf, fp );

        // Reached EoF
        if ( feof( fp ) )
           break;

        // If line begins with #, ignore it
        if ( strncmp( str, "#", 1 ) == 0 )
            continue;

        // Get the first token
        token = strtok( str, WHITESPACE );

        // Check if token is a shell command
        if ( shellCommands( token ) )
            continue;

        // Get start time
        gettimeofday( &tvalStart, NULL );

        int id;
        bool shouldExec = true;
        int argCount = 0;
        char *argVector[ARG_MAX];

        // Child
        switch ( id = fork() ) {

        case -1:

            perror( "Fork" );
            return -1;

        case 0:

            // Gets tokens one by one
            while ( token != NULL ) {

                // First see if the token is a redirection
                int retVal = ioRedirect(token);

                // If any redirection fails, exit with an error status
                if( retVal == -1 ) {

                    shouldExec = false;
                    lastExitStatus = 1;
                    break;

                }
                // If ioRedirect() returns 1, it didn't fail, but the token was not a redirect instruction
                else if( retVal == 1 ) {

                    argVector[argCount] = token;
                    argCount++;

                }

                // Get next token
                token = strtok( NULL, WHITESPACE );

            }

            if( shouldExec ) {

                int err = execvp( argVector[0], argVector );

                // Exec returns -1 on error
                if ( err == -1 ) {

                    fprintf( stderr, "Exec for command %s failed.\n", argVector[0] );
                    perror( "Error" );
                    lastExitStatus = 127;

                }

            }

            // Clean up the child
            fclose(fp);
            return lastExitStatus;

        default:

            // Get end time
            wait3( &ws, 0, &usage );
            gettimeofday( &tvalEnd, NULL );

        }

        // Get exit status
        fprintf( stderr, "Child process %i exited ", id );
        if ( WIFEXITED( ws ) ) {

            lastExitStatus = WEXITSTATUS( ws );

            if ( lastExitStatus == 0 )
                fprintf( stderr, "normally\n" );
            else if ( lastExitStatus > 0 )
                fprintf( stderr, "with return value %i\n", lastExitStatus );

        }
        else if ( WIFSIGNALED( ws ) ) {

            lastExitStatus = WTERMSIG( ws );
            fprintf( stderr, "with signal %i (%s)\n", lastExitStatus, strsignal( lastExitStatus ) );
            lastExitStatus += 128; // Update signal exit status

        }

        // Measure time
        fprintf( stderr, "\nReal Time: %f(s) ",
            ( (double)( tvalEnd.tv_usec - tvalStart.tv_usec ) / 1000000
            + (double)( tvalEnd.tv_sec - tvalStart.tv_sec ) ) );
        fprintf( stderr, "User Time: %f(s) ",
            ( (double)( usage.ru_utime.tv_usec ) / 1000000
            + (double)( usage.ru_utime.tv_sec ) ) );
        fprintf( stderr, "System Time: %f(s)\n\n",
            ( (double)( usage.ru_stime.tv_usec ) / 1000000
            + (double)( usage.ru_stime.tv_sec ) ) );

    }

    // Clean up parent
    fclose(fp);
    fprintf( stderr, "Reached EoF. Exiting with exit code %i\n", lastExitStatus );
    return lastExitStatus;

}


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
            fprintf( stderr, "Current working directory: %s\n\n", currentDir );
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

		if ( ( fd=open(filePath, O_RDONLY, 0666) ) < 0 ) {

			fprintf( stderr, "Couldn't open file (%s) while redirecting STDIN\n", filePath );
			perror( "Error" );
			return -1;

		}
		if ( dup2(fd,0) < 0 ) {

            fprintf( stderr, "Couldn't dup2 fd of file (%s) to STDIN\n", filePath );
			perror( "Error" );
			return -1;

		}
		close(fd);

		return 0;

	}

    // >>filename: redirect stdout (APPEND)
	if( strncmp( token, ">>", 2 ) == 0 ) {

		memmove( filePath, filePath+2, strlen(filePath) );

		if ( ( fd=open(filePath, O_WRONLY|O_CREAT|O_APPEND, 0666) ) < 0 ) {

            fprintf( stderr, "Couldn't open file (%s) while redirecting STDOUT (APPEND)\n", filePath );
            perror( "Error" );
			return -1;
		}
		if ( dup2(fd,1) < 0 ) {

			fprintf( stderr, "Couldn't dup2 fd of file (%s) to STDOUT (APPEND)\n", filePath );
			perror( "Error" );
			return -1;

		}
		close(fd);

		return 0;

	}

    // >filename: redirect stdout (TRUNC)
	if( strncmp( token, ">", 1 ) == 0 ) {

		memmove( filePath, filePath+1, strlen(filePath) );

		if ( ( fd=open(filePath, O_WRONLY|O_CREAT|O_TRUNC, 0666) ) < 0 ) {

			fprintf( stderr, "Couldn't open file (%s) while redirecting STDOUT (TRUNC)\n", filePath );
			perror( "Error" );
			return -1;

		}
		if ( dup2(fd,1) < 0 ) {

			fprintf( stderr, "Couldn't dup2 fd of file (%s) to STDOUT (TRUNC)\n", filePath );
			perror( "Error" );
			return -1;

		}
		close(fd);

		return 0;

	}

    // 2>>filename: redirect stderr (APPEND)
	if( strncmp( token, "2>>", 3 ) == 0 ) {

		memmove( filePath, filePath+3, strlen(filePath) );

		if ( ( fd=open(filePath, O_WRONLY|O_CREAT|O_APPEND, 0666) ) < 0 ) {

			fprintf( stderr, "Couldn't open file (%s) while redirecting STDERR (APPEND)\n", filePath );
			perror( "Error" );
			return -1;

		}
		if ( dup2(fd,2) < 0 ) {

			fprintf( stderr, "Couldn't dup2 fd of file (%s) to STDERR (APPEND)\n", filePath );
			perror( "Error" );
			return -1;

		}
		close(fd);

		return 0;

	}


    // 2>filename: redirect stdout (TRUNC)
	if( strncmp( token, "2>", 2 ) == 0 ) {

		memmove( filePath, filePath+2, strlen(filePath) );

		if ( ( fd=open(filePath, O_WRONLY|O_CREAT|O_TRUNC, 0666) ) < 0 ) {

			fprintf( stderr, "Couldn't open file (%s) while redirecting STDERR (TRUNC)\n", filePath );
			perror( "Error" );
			return -1;
		}
		if ( dup2(fd,2) < 0 ) {

			fprintf( stderr, "Couldn't dup2 fd of file (%s) to STDERR (TRUNC)\n", filePath );
			perror( "Error" );
			return -1;

		}
		close(fd);

		return 0;

	}

	// Return value of 1 indicates no error and no redirection (the token is an argument)
	return 1;

}
