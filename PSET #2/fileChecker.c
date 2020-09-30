#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define SIZE_ARR 4096

// Recursive function
int fileChecker( char* filename );

// Global ints to count each diff files
int numFIFO = 0, numFCHR = 0, numFDIR = 0, numFBLK = 0, numFREG = 0, numFLNK = 0, numFSOCK = 0;
int numlink = 0, sumOfSize = 0, sumOfBlock = 0, badFileName = 0, numSymlink = 0;

int main( int argc, char *argv[] ) {

    // If there aren't 2 arguments (function + pathname), print error
    char* start;
    if (argc != 2) {

        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        return -1;
    }
    else
        start = argv[1];

    // Start recursion
    fileChecker(start);

    printf("# of block device inodes:               %d.\n", numFBLK);
    printf("# of character device inodes:           %d.\n", numFCHR);
    printf("# of directory inodes:                  %d.\n", numFDIR);
    printf("# of FIFO/pipe inodes:                  %d.\n", numFIFO);
    printf("# of symlink inodes:                    %d.\n", numFLNK);
    printf("# of regular file inodes:               %d.\n", numFREG);
    printf("# of sock inodes:                       %d.\n", numFSOCK);
    printf("# of directory with problematic names:  %d.\n", badFileName);
    printf("Total size of all regular files:        %d.\n", sumOfSize);
    printf("Total disk blocks allocated:            %d.\n", sumOfBlock);
    printf("# of inodes with nlink > 1:             %d.\n", numlink);
    printf("# of valid symlinks:                    %d.\n", numSymlink);

    return 0;

}

int fileChecker( char* direc ) {

    DIR *dir;
    struct dirent *entry;
    struct stat sb;

    // Cannot open directory
    if ( !( dir = opendir(direc) ) ) {

        fprintf(stderr, "Warning: Cannot open directory %s for reading: %s\n", direc, strerror(errno));
        return -1;

    }

    // Read directory
    while ( ( entry = readdir(dir) ) != NULL ){

        char path[SIZE_ARR];

        // Store each entry to path
        snprintf( path, sizeof(path), "%s/%s", direc, entry->d_name );

        // Check if you can check stat
        if ( stat(path, &sb) < 0 )
            fprintf( stderr, "Stat can not be shown %s: %s\n", path, strerror(errno) );

        // Check if nlink > 1
        if ( ( ( sb.st_mode & S_IFMT ) != S_IFDIR ) && ( sb.st_nlink > 1 ) )
            numlink++;

        // Check the filename
        char *name = entry->d_name;
        for ( int i=0; i < strlen( name ); i++ ) {

            if( name[i] == '\'' || name[i] == '~' || !isprint( name[i] ) || isspace( name[i] ) ) {

                badFileName++;
                break;

            }

        }

        printf( "File type: " );

        //checking each different cases, add the required stuff if you can (total number of diff cases, etc)
        switch ( sb.st_mode & S_IFMT ) {

            case S_IFREG:

                //sum of the size and blocks
                sumOfSize += sb.st_size;
                sumOfBlock += sb.st_blocks;

                printf( " Regular File\n" );
                numFREG++;
                break;

             case S_IFLNK:

                printf( " Symbolic Link\n" );
                //check for valid symlinks
                if( access( path, F_OK ) == 0 )
                    numSymlink++;
                else
                    printf( "Invalid symbolic link with path: %s\n", path );

                numFLNK++;
                break;

            case S_IFDIR:

                printf( " Directory\n" );
                numFDIR++;

                // If the path is not . and ..
                // Then continue recursion
                if ( !strcmp( entry->d_name, "." ) && !strcmp( entry->d_name, ".." ) )
                    fileChecker(path);

                break;

            case S_IFCHR:

                printf( " Character Special\n" );
                numFCHR++;
                break;

            case S_IFBLK:

                printf( " Block Special\n" );
                numFBLK++;
                break;

            case S_IFIFO:

                printf( " FIFO or pipe\n" );
                numFIFO++;
                break;

            default:

                printf( "Unix unique files, %s", entry->d_type );
                break;

        }

    }

    return 0;

}
