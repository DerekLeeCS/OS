#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#define STR_STDIN  "0"
#define STR_STDOUT "1"
#define STR_STDERR "2"
#define SIZE_BUF   4096
extern int errno;
char buf[SIZE_BUF];
int kitty( char*, int );
int main( int argc, char** argv ) {
  char* files[ argc ];
  int fileCount = 0;
  char* fileOut = "";
  int fdOut, errNum;
  int i = 1;
  if ( argc >= 3 ) {
    if ( argv[i][0] == '-' && argv[i][1] == 'o' ) {
      fileOut = argv[i+1];
      i += 2;
    }
  }
  if ( fileOut != "" ) {
    fdOut = open( fileOut, O_WRONLY | O_CREAT | O_TRUNC, 0666 );
    if ( fdOut == -1 ) {
      fprintf( stderr, "Cannot open output file: %s\n", fileOut );
      perror( "Error:" );
      return -1;
    }
  }
  else
    fdOut = STDOUT_FILENO;
  for ( ; i<argc; i++ ) {
    if ( argv[i][0] == '-' ) {
      if ( argv[i][1] == 'o' ) {
        fprintf( stderr, "Error: Invalid argument for output file.\n" );
        return -1;
      }
      int fdTemp = open( argv[i], O_RDONLY );
      if ( fdTemp >= 0 ) {
        files[ fileCount++ ] = argv[i];
        if ( close( fdTemp ) < 0 ) {
          fprintf( stderr, "Error: Cannot close input file: %s\n", argv[i] );
          perror( "Error" );
          return -1;
        }
      }
      else
        files[ fileCount++ ] = STR_STDIN;
    }
    else
      files[ fileCount++ ] = argv[i];
  }
  if ( fileCount == 0 )
    files[ fileCount++ ] = STR_STDIN;
  int kittyResult;
  for ( i=0; i<fileCount; i++ ) {
    kittyResult = kitty( files[i], fdOut );
    if ( kittyResult < 0 ) {
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
          fprintf( stderr, "Cannot close input file: %s\n", files[i] );
          break;
      }
      perror( "Error" );
      return -1;
    }
  }
  if ( fdOut != STDOUT_FILENO ) {
    if ( close ( fdOut ) < 0 ) {
      fprintf( stderr, "Cannot close output file: %s\n", fileOut );
      perror( "Error" );
      return -1;
    }
  }
  return 0;
}
int kitty( char* fileIn, int fdOut ) {
  int fdIn, szRead = 1, szWrite, szTransferred = 0;
  int numReadCalls = 0, numWriteCalls = 0;
  bool isBinary = false;
  if ( fileIn != STR_STDIN ) {
    fdIn = open( fileIn, O_RDONLY );
    if ( fdIn == -1 )
      return -1;
  }
  else
    fdIn = STDIN_FILENO;
  while (1) {
    szRead = read( fdIn, buf, SIZE_BUF );
    numReadCalls++;
    if ( szRead >= 0 ) {
      if ( szRead == 0 )
        break;
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
      if ( szWrite >= 0 ) {
        szTransferred += szWrite;
        if ( szWrite < SIZE_BUF )
          break;
      }
      else
        return -3;
      if ( szRead < SIZE_BUF )
        break;
    }
    else
      return -2;
  }
  if ( fdIn != STDIN_FILENO ) {
    if ( close( fdIn ) < 0 )
      return -4;
  }
  fprintf( stderr, "Transferred %d bytes and made %d read system call(s) and %d write system call(s).\n",
           szTransferred, numReadCalls, numWriteCalls );
  if ( isBinary ) {
    fprintf( stderr, "Warning: Binary file encountered: " );
    if ( fdIn == STDIN_FILENO )
      fprintf( stderr, "<standard input>\n" );
    else
      fprintf( stderr, "%s\n", fileIn );
  }
  return 0;
}
