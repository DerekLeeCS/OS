// Derek Lee, Steven Lee, Shine Li
// ECE-357 Computer Operating Systems
// PSET #1 Question #3

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


// Used for comprehension
#define STR_STDIN  "0"
#define STR_STDOUT "1"
#define STR_STDERR "2"
#define SIZE_BUF   4096


extern int errno;
char buf[SIZE_BUF];

int kitty( char*, int );


int main( int argc, char** argv ) {

  char* files[ argc-1 ]; // Array of strings
  int fileCount = 0;     //  Used to increment files[]
  char* fileOut = "";    // Output string

  int fdOut, errNum, opt;
  // i = 0 is kitty
  int i = 1;

  // Check if output file is specified
  if ( argc >= 2 ) {

    if ( argv[i][0] == '-' && argv[i][1] == 'o' ) {

        fileOut = argv[i+1];
        i += 2;

    }

  }

  // Iterate through the rest of the arguments
  for ( ; i<argc; i++ ) {

    // Check if "-" is a file or STDIN
    if ( strcmp( argv[i], "-" ) == 0 ) {

      // Check if "-" is a file
      int fdTemp = open( argv[i], O_RDONLY );
      if ( fdTemp >= 0 ) {

        files[ fileCount++ ] = argv[i];

        // In case there is an error closing the file
        if ( close( fdTemp ) < 0 ) {

          fprintf( stderr, "Error: Cannot close input file: %s\n", argv[i] );
          perror( "Error" );
          return -1;

        }

      }
      // STDIN
      else   files[ fileCount++ ] = STR_STDIN;

    }
    // Regular input file
    else   files[ fileCount++ ] = argv[i];

  }

  // If no input file specified, read from STDIN
  if ( fileCount == 0 )   files[ fileCount++ ] = STR_STDIN;

  // Open output file if not STDOUT
  // Otherwise, use STDOUT
  if ( fileOut != "" ) {

    fdOut = open( fileOut, O_WRONLY | O_CREAT | O_TRUNC, 0666 );
    if ( fdOut == -1 ) {

      fprintf( stderr, "Cannot open output file: %s\n", fileOut );
      perror( "Error" );
      return -1;

    }

  }
  else   fdOut = STDOUT_FILENO;


  int kittyResult;

  // Iterate through input files
  for ( int i=0; i<fileCount; i++ ) {

    kittyResult = kitty( files[i], fdOut );

    // Terminate on error
    if ( kittyResult < 0 ) {

      // Print error information
      switch ( kittyResult ) {

        case ( -1 ):

          fprintf( stderr, "Cannot open input file for reading: %s\n", files[i] );
          break;

        case ( -2 ):

          fprintf( stderr, "Cannot read from input file: %s\n", files[i] );
          break;

        case ( -3 ):

          fprintf( stderr, "Cannot write to output file: %s\n", fileOut );
          break;

        case ( -4 ):

          fprintf( stderr, "Partial write encountered\n" );
          break;

        case ( -5 ):

          fprintf( stderr, "Cannot close input file: %s\n", files[i] );
          break;

      }

      perror( "Error" );
      return -1;

    }

  }

  // Close output file
  if ( fdOut != STDOUT_FILENO ) {

    if ( close ( fdOut ) < 0 ) {

      fprintf( stderr, "Cannot close output file: %s\n", fileOut );
      perror( "Error" );
      return -1;

    }

  }

  return 0;

}


// Returns -1 for opening input file error
// Returns -2 for read error
// Returns -3 for write error
// Returns -4 for partial write error
// Returns -5 for closing input file error
int kitty( char* fileIn, int fdOut ) {

  int fdIn, szRead = 1, szWrite, szTransferred = 0;
  int numReadCalls = 0, numWriteCalls = 0;
  bool isBinary = false;

  // Open inFile if not STDIN
  if ( fileIn != STR_STDIN ) {

    fdIn = open( fileIn, O_RDONLY );

    // Opening input file error
    if ( fdIn == -1 )   return -1;

  }
  else   fdIn = STDIN_FILENO;

  // Read and write
  while (1) {

    szRead = read( fdIn, buf, SIZE_BUF );
    numReadCalls++;

    // No error
    if ( szRead > 0 ) {

      // Parse for binary
      if ( !isBinary ) {

        for ( int i=0; i<szRead; i++ ) {

          if ( !( isprint( buf[i] ) | isspace( buf[i] ) ) ) {

            isBinary = true;
            break;

           }

        }

      }

      szWrite = write( fdOut, buf, szRead );
      numWriteCalls++;
      if ( szWrite != szRead )   return -4;

      // No error
      // Writing 0 bytes should be an error
      if ( szWrite > 0 )   szTransferred += szWrite;

      // Write Error
      else   return -3;

    }
    // Reached EoF
    else if ( szRead == 0 )   break;
    // Read Error
    else   return -2;

  }

  // Do not close STDIN
  if ( fdIn != STDIN_FILENO ) {

    // Error while closing
    if ( close( fdIn ) < 0 )   return -5;

  }

  fprintf( stderr, "Transferred %d bytes and made %d read system call(s) and %d write system call(s).\n",
           szTransferred, numReadCalls, numWriteCalls );

  if ( isBinary ) {

    fprintf( stderr, "Warning: Binary file encountered: " );
    if ( fdIn == STDIN_FILENO )   fprintf( stderr, "<standard input>\n" );
    else   fprintf( stderr, "%s\n", fileIn );

  }

  return 0;

}


