#include "types.h"
#include "stat.h"
#include "user.h"
// #include <file.h>

#define STD_OUT 1
#define TRUE 1
#define FALSE 0

// should be able to include fcntl.h, but getting errors - TODO
#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200

//hardcoded for now
char last_line[6];
char this_line[6];

void usage(int fd_out) {
    printf(fd_out, "usage: uniq input-file-name");
}

void uniq(int fd_in, int count, int duplicate, int ignore) {
    
    //while read in line exists
    int infile_num_bytes, n;
    if ((n = read(fd_in, last_line, sizeof(last_line)) <= 0)) {
        printf(STD_OUT, "error - file empty");
        exit();
    }

    while ((infile_num_bytes = read(fd_in, this_line, sizeof(this_line))) > 0) { // consider read() wrapper for readline()
        // TODO - need to account for a line being longer than 512 bytes. 

        // if dupe exists, just leave last_line as is
        // and continue. Otherwise, print last line, 
        // set last_line to this line, and continue.
        if (strcmp(last_line, this_line) != 0) {
            // dupe not found
            printf(STD_OUT, "%s", last_line);
            strcpy(last_line, this_line);
        }
    }
}

int main(int argc, char *argv[]) {
    int in_fd;
    // Flag definitions TODO - consider using bitwise logic to save on memory...
    int count, duplicate, ignore;
    count = duplicate = ignore = 0;

    if (argc <=1) {
        usage(STD_OUT);
        exit();
    }
    else {
        // argv[0] is uniq
        // argv[1] is input file name
        in_fd = open(argv[1], O_RDONLY);
        // TODO - implement -c, -d, -i flags with case statement block
    }

    uniq(in_fd, count, duplicate, ignore);
    exit();
    return 0;
}