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
int go_thru( char* filename );

// Global ints
int numFIFO, numFCHR, numFDIR, numFBLK, numFREG, numFLNK, numFSOCK;
int numlink, sumOfSize, sumOfBlock, problem_name, numSymlink;
numFIFO = numFCHR = numFDIR = numFBLK = numFREG = numFLNK = numFSOCK = 0;
numlink = sumOfSize = sumOfBlock = problem_name = numSymlink = 0;

int main( int argc, char *argv[] ) {

    char* start;
    if (argc != 2) {

        fprintf( stderr, "Usage: %s <pathname>\n", argv[0] );
        return -1;
    }
    else
        start = argv[1];

    go_thru(start);

    printf("# of block device inodes:               %d.\n", numFBLK);
    printf("# of character device inodes:           %d.\n", numFCHR);
    printf("# of directory inodes:                  %d.\n", numFDIR);
    printf("# of FIFO/pipe inodes:                  %d.\n", numFIFO);
    printf("# of symlink inodes:                    %d.\n", numFLNK);
    printf("# of regular file inodes:               %d.\n", numFREG);
    printf("# of sock inodes:                       %d.\n", numFSOCK);
    printf("# of directory with problematic names:  %d.\n", problem_name);
    printf("Total size of all regular files:        %d.\n", sumOfSize);
    printf("Total disk blocks allocated:            %d.\n", sumOfBlock);
    printf("# of inodes with nlink > 1:             %d.\n", numlink);
    printf("# of valid symlinks:                    %d.\n", numSymlink);

    return 0;

}

int go_thru( char* direc ) {

    DIR *dir;
    struct dirent *entry;
    struct stat sb;

    // Cannot open directory
    if ( !( dir = opendir(direc) ) ) {

        fprintf(stderr, "Warning: Cannot open directory %s for reading: %s\n", direc, strerror(errno));
        return -1;

    }

    while ( ( entry = readdir(dir) ) != NULL ){

        char path[SIZE_ARR];
        snprintf( path, sizeof(path), "%s/%s", direc, entry->d_name );

        if (stat(path, &sb) < 0)
            fprintf( stderr, "Stat can not be shown %s: %s\n", path, strerror(errno) );

        if ( ( ( sb.st_mode & S_IFMT ) != S_IFDIR ) && ( sb.st_nlink > 1 ) )
            numlink++;

        if ( strstr( entry->d_name, '\'' ) || strstr( entry->d_name, "~" ) || !isprint(entry->d_name ) || isspace( entry->d_name ) )
            problem_name++;

        printf( "File type: " );
        //checking each different cases, add the required stuff if you can (total number of diff cases, etc)
        switch ( sb.st_mode & S_IFMT ) {

            case S_IFREG:

                sumOfSize += sb.st_size;
                sumOfBlock += sb.st_blocks;
                printf( " Regular File\n" );
                numFREG++;
                break;

             case S_IFLNK:

                printf( " Symbolic Link\n" );
                if( access( path, F_OK ) == 0 )
                    numSymlink++;
                else
                    printf( "Invalid symbolic link with path: %s\n", path );

                numFLNK++;
                break;

            case S_IFDIR:

                printf( " Directory\n" );
                numFDIR++;

                if ( ( entry->d_name != "." ) || ( entry->d_name != ".." ) ){
                    go_thru(path);
                }
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
 /*case S_IFSOCK:
                break;
            case S_IFMPC:

                break;
            case S_IFNAM:

                break;
            case S_IFMPB:

                break;
            case S_IFCMP:

                break;
            case S_IFSHAD:

                break;
            case S_IFDOOR:

                break;
            case S_IFWHT:

                break;
            case S_IFPORT:

                break;
            */

        /*if(entry->d_type == DT_DIR){
            type+=1;

            //check for the . and .. and ignore them
            if ((entry->d_name == ".") && (entry->d_name == "..")){
            }
            else{
                go_thru(path);
            }
        }
        else if(entry->d_type == DT_REG){
            printf("regular file\n");
            sumOfSize += sb.st_size;
            sumOfBlock += sb.st_blocks;
            if(sb.st_nlink>1){
                nlink1++;
        }
        else if(entry->d_type == DT_LNK){
            if((readlink(path, link, SIZE_ARR) < 0) {
                fprintf(stderr, "Cannot read symlink %s: %s\n", path, strerror(errno));
                return -1;
        }
        else{

        }*/
