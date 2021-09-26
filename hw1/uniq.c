#include "types.h"
#include "stat.h"
//#include <fcntl.h>
#include "user.h"

#define STD_OUT 1
#define TEMP_FILE_NAME "temp_file.txt"
#define TRUE 1
#define FALSE 0

// should be able to include fcntl.h, but getting errors - TODO
#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200

//hardcoded for now
char infile_buf[6];
char tempfile_buf[6];
// int string_compare(char *str1, char *str2, int ignore_case)

// int line_exists(int temp_file_descriptor, char *search_line)

// void write_line(int temp_file_descriptor, char *write_line)

void usage(int fd_out) {
    printf(fd_out, "usage: uniq input-file-name");
}

void uniq(int fd_in, int count, int duplicate, int ignore) {
    int state = mknod(TEMP_FILE_NAME, 0777, 0); // not working properly
    printf(STD_OUT, "%d", state);
    if (state < 0) {
        printf(STD_OUT, "mknod() error");
        exit();
    }
    int temp_fd = open(TEMP_FILE_NAME, O_RDWR); // open temp file
    if (temp_fd == -1) {
        printf(STD_OUT, "error w temp file!");
        exit();
    } 
    
    //while read in line exists
    int infile_num_bytes, tempfile_num_bytes;
    while ((infile_num_bytes = read(fd_in, infile_buf, sizeof(infile_buf))) > 0) { // consider read() wrapper for readline()
        printf(STD_OUT, "line read!");
        // get a line of file from bytes.
        // TODO - need to account for a line being longer than 512 bytes. 


        // check if line in temp file using string_compare
        // if line isn't in temp file, write line to temp file
        int dupe = FALSE;
        while ((tempfile_num_bytes = read(temp_fd, tempfile_buf, sizeof(tempfile_buf))) > 0) {
            if (strcmp(infile_buf, tempfile_buf) == 0) {
                printf(STD_OUT, "dupe found!");
                dupe = TRUE;
            }
        }

        if (!dupe) { // no dupes found
            write(temp_fd, tempfile_buf, sizeof(tempfile_buf));
        }
    }
    char printf_buf[512];
    int n;
    while ((n = read(temp_fd, printf_buf, sizeof(printf_buf))) > 0) { 
        printf(STD_OUT, printf_buf);
    }
    // delete temp file
    //write to stdout
    unlink(TEMP_FILE_NAME);
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