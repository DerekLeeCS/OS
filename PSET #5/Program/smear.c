#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

void changeFile( char*, char*, char* );

int main( int argc, char* argv[] ) {

	char *stringTarget, *stringReplacement;

	if ( argc < 4 ) {

		fprintf( stderr, "Incorrect argument format\n." );
		exit( EXIT_FAILURE );

	}

	// Copy argv's to variables
	char* fileNames[ argc-3 ];

	stringTarget = argv[1];
	stringReplacement = argv[2];

	for ( int i=3; i<argc; i++ )
		fileNames[ i-3 ] = argv[i];

	// Loop through every file
	int fd, sz;
	char* addr;
	struct stat st;

	for ( int i=0; i<argc-3; i++ ) {

		// Open the file
		fd = open( fileNames[i], O_RDWR );
		if ( fd == -1 ) {

			fprintf( stderr, "Error: Cannot open file (%s) for reading and writing.\n", fileNames[i] );
			perror( "Error" );
			exit( EXIT_FAILURE );

		}

		// Get size of file
		if ( fstat( fd, &st ) < 0 ) {

			fprintf( stderr, "Error: Cannot open file (%s) to get stats.\n", fileNames[i] );
			perror( "Error" );
			exit( EXIT_FAILURE );

		}
		sz = st.st_size;

		// Map the file
		addr = mmap( NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
		if ( addr == MAP_FAILED ) {

			fprintf( stderr, "Error: Cannot map file (%s).\n", fileNames[i] );
			exit( EXIT_FAILURE );

		}

		// Ensure fclose() properly closed the file
		if( close( fd ) < 0 ) {

			fprintf( stderr, "Error: Cannot close file (%s).\n", fileNames[i] );
			perror( "Error" );
			exit( EXIT_FAILURE );

		}

		changeFile( stringTarget, stringReplacement, addr );

		// Unmap
		if ( munmap( addr, sz ) < 0 ) {

			fprintf( stderr, "Error: Cannot unmap file (%s).\n", fileNames[i] );
			perror( "Error" );
			exit( EXIT_FAILURE );

		}

	}

	return 0;

}

void changeFile( char* stringTarget, char* stringReplacement, char* addr ) {

	char* curPos = addr;
	size_t n = strlen( stringReplacement );

	// Finds all instances of the string, even substrings
	while( ( curPos = strstr( curPos, stringTarget ) ) != NULL ) {

		strncpy( curPos, stringReplacement, n );
		curPos += n;

	}

	return;

}

