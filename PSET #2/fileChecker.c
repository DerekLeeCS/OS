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


// Custom struct to store information
struct info {

    // Stores volume of inode to ensure recursive search remains in the same volume
    int volume = NULL;

    // Stores inode numbers of inodes with multiple hardlinks
    // Prevents checking an inode with multiple hardlinks more than once
    int *multiNode;

};

// Recursive function
int fileChecker( char* filename );

// Global ints to keep track of counts
int numFIFO = 0, numFCHR = 0, numFDIR = 0, numFBLK = 0, numFREG = 0, numFLNK = 0, numFSOCK = 0;
int numlink = 0, sumOfSize = 0, sumOfBlock = 0, badFileName = 0, numSymlink = 0;

// Global struct
struct info globInfo;

int main( int argc, char *argv[] ) {

    char* start;

    // If there aren't 2 arguments (function + pathname), print error
    if (argc != 2) {

        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        return -1;
    }
    else
        start = argv[1];

    // Allocate size of multiNode array
    // Can store 10000 inodes
    globInfo.multiNode = malloc( 10000 * sizeof( int ) );

    // Start recursion
    fileChecker(start);

    // a) # of inodes of each type (e.g. directory, file, symlink, etc.) encountered
    printf("# of block device inodes:                       %d.\n", numFBLK);
    printf("# of character device inodes:                   %d.\n", numFCHR);
    printf("# of directory inodes:                          %d.\n", numFDIR);
    printf("# of FIFO/pipe inodes:                          %d.\n", numFIFO);
    printf("# of symlink inodes:                            %d.\n", numFLNK);
    printf("# of regular file inodes:                       %d.\n", numFREG);
    printf("# of sock inodes:                               %d.\n", numFSOCK);
    // b) for all regular files encountered, the sum of their sizes,
    // and the sum of the number of disk blocks allocated for them
    printf("Total size of all regular files:                %d.\n", sumOfSize);
    printf("Total disk blocks allocated:                    %d.\n", sumOfBlock);
    // c) # of inodes (except, of course, directories) which have a link count of more than 1
    printf("# of inodes with nlink > 1:                     %d.\n", numlink);
    // d) # of symlinks encountered that did not (at least at the time of the exploration) resolve to a valid target
    printf("# of invalid symlinks:                          %d.\n", numSymlink);
    // e) # of directory entries encountered which would be "problematic" to enter at the keyboard
    printf("# of directory entries with problematic names:  %d.\n", badFileName);

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

        // Check if file cannot be accessed
        if ( access( path, R_OK ) != 0 ) {

            fprintf( stderr, "Cannot access file (%s).\n", path );
            perror( "Error" );
            return -1;

        }

        // Check if cannot read information about file
        if ( lstat(path, &sb) < 0 ) {

            fprintf( stderr, "Cannot read information about file (%s).\n", path );
            perror( "Error" );
            return -1;

        }

        // If globInfo doesn't have a volume, assign current volume
        // Should only happen for initial call to fileChecker()
        if ( globInfo.volume == NULL )
            globInfo.volume = sb.st_dev;
        // Check if inode is in the same volume
        else if ( globInfo.volume != sb.st_dev ) {

            fprintf( stderr, "Attempting to access an inode (%s) in a different volume.\n", path );
            perror( "Error" );
            return -1;

        }

        // Check if nlink > 1
        if ( ( ( sb.st_mode & S_IFMT ) != S_IFDIR ) && ( sb.st_nlink > 1 ) ) {

            // Check if inode was previously encountered
            for ( int i=0; i<numlink; i++ ) {

                if ( globInfo.multiNode[i] == sb.st_ino )
                    return -1;

            }

            // Adds inode number to array of inodes with multiple hardlinks
            globInfo.multiNode[ numLink ] = sb.st_ino;
            numlink++;

        }

        // Check the filename
        char *name = entry->d_name;
        for ( int i=0; i < strlen( name ); i++ ) {

            if( name[i] == '\'' || name[i] == '~' || !isprint( name[i] ) || isspace( name[i] ) ) {

                badFileName++;
                break;

            }

        }

        // Check each cases
        // Add the required stuff if you can (total number of diff cases, etc)
        switch ( sb.st_mode & S_IFMT ) {

            // Regular File
            case S_IFREG:

                // Add the size and blocks
                sumOfSize += sb.st_size;
                sumOfBlock += sb.st_blocks;

                numFREG++;
                break;

            // Symbolic Link
            case S_IFLNK:

                // Resolve symlink to its target
                // If target is invalid
                if( stat( path, &sb ) < 0 ) {

                    fprintf( stderr, "Symlink (%s) did not resolve to a valid target.\n", path );
                    perror( "Error" );
                    numSymlink++;

                }

                numFLNK++;
                break;

            // Directory
            case S_IFDIR:

                numFDIR++;

                // If the path is not . and ..
                // Then continue recursion
                if ( !strcmp( entry->d_name, "." ) && !strcmp( entry->d_name, ".." ) )
                    fileChecker(path);

                break;

            // Character Special
            case S_IFCHR:

                numFCHR++;
                break;

            // Block Special
            case S_IFBLK:

                numFBLK++;
                break;

            // FIFO or pipe
            case S_IFIFO:

                numFIFO++;
                break;

            // Unix unique files
            default:

                printf( stderr, "File (%s) is unknown inode type (%s)", path, entry->d_type );
                break;

        }

    }

    return 0;

}
