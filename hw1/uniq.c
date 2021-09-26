#include "types.h"
#include "stat.h"
#include "user.h"

#define STD_OUT 1
#define TRUE 1
#define FALSE 0
// should be able to include fcntl.h, but getting errors - TODO
#define O_RDONLY  0x000

//hardcoded for now
char last_line[6];
char this_line[6];

void usage(int fd_out) {
    printf(fd_out, "usage: uniq input-file-name -w [OPTIONAL] -i [OPTIONAL] -c [OPTIONAL]");
    exit();
}

void to_upper(char *str, char *copy_str) {
    int i;
    for (i = 0; str[i]!='\0'; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            copy_str[i] = str[i] -32;
        } else {
            copy_str[i] = str[i];
        }
    }
}

void uniq_duplicate_mode(int fd_in, int ignore, char * last_line) {
    // only print duplicates
    int infile_num_bytes;
    int dupe_found = FALSE;
    while ((infile_num_bytes = read(fd_in, this_line, sizeof(this_line))) > 0) { 
        // TODO - need to account for a line being longer than 512 bytes - readline() wrapper for read()

        // if dupe exists, just leave last_line as is
        // and continue. Otherwise, print last line, 
        // set last_line to this line, and continue.
        char last_line_cmp[strlen(last_line)];
        char this_line_cmp[strlen(this_line)];
        if (ignore) {
            to_upper(last_line, last_line_cmp);
            to_upper(this_line, this_line_cmp);
        } else {
            strcpy(last_line_cmp, last_line);
            strcpy(this_line_cmp, this_line);
        }

        // if we find a dupe, and its the first occurence, print it and set
        // the dupe flag. Otherwise, set the dupe flag to false.
        if (strcmp(last_line_cmp, this_line_cmp) == 0 && !dupe_found) {
            printf(STD_OUT, "%s", last_line);
            dupe_found = TRUE;
        } else {
            strcpy(last_line, this_line);
            dupe_found = FALSE;
        }
    }
    // if dupe exists, just leave last_line as is
    // and continue. Otherwise, print last line, 
    // set last_line to this line, and continue.
    char last_line_cmp[strlen(last_line)];
    char this_line_cmp[strlen(this_line)];
    if (ignore) {
        to_upper(last_line, last_line_cmp);
        to_upper(this_line, this_line_cmp);
    } else {
        strcpy(last_line_cmp, last_line);
        strcpy(this_line_cmp, this_line);
    }
    if (strcmp(last_line_cmp, this_line_cmp) == 0 && !dupe_found) {
        printf(STD_OUT, "%s", last_line);
    }
}

void uniq_non_duplicate_mode(int fd_in, int ignore, int count, char * last_line) {
    int dupe_count = 1;
    int infile_num_bytes;
    while ((infile_num_bytes = read(fd_in, this_line, sizeof(this_line))) > 0) { 
        // TODO - need to account for a line being longer than 512 bytes - readline() wrapper for read()

        // if dupe exists, just leave last_line as is
        // and continue. Otherwise, print last line, 
        // set last_line to this line, and continue.
        char last_line_cmp[strlen(last_line)];
        char this_line_cmp[strlen(this_line)];
        if (ignore) {
            to_upper(last_line, last_line_cmp);
            to_upper(this_line, this_line_cmp);
        } else {
            strcpy(last_line_cmp, last_line);
            strcpy(this_line_cmp, this_line);
        }

        if (strcmp(last_line_cmp, this_line_cmp) != 0) {
            // dupe not found
            if (count) {
                printf(STD_OUT, "%d %s", dupe_count, last_line);
            } else {
                printf(STD_OUT, "%s", last_line);
            }
            strcpy(last_line, this_line);
            dupe_count = 1;
        } else {
            dupe_count++;
        }

    }
    // Always print last line. If no dupe found on last 2 lines, 
    // only the 2nd to last will have printed. Alternatively, if dupe was found,
    // the duplicated line needs to be printed here.
    if (count) {
        printf(STD_OUT, "%d %s", dupe_count, last_line);
    } else {
        printf(STD_OUT, "%s", last_line);
    }
}

void uniq(int fd_in, int count, int duplicate, int ignore) {
    // Proxy Method for choosing to run uniq in duplicate or non-duplicate mode.
    int n;
    if ((n = read(fd_in, last_line, sizeof(last_line)) <= 0)) {
        printf(STD_OUT, "error - file empty");
        exit();
    }

    switch (duplicate) {
        case 0 :
            uniq_non_duplicate_mode(fd_in, ignore, count, last_line);
        case 1 :
            uniq_duplicate_mode(fd_in, ignore, last_line);
    }
        
}

int main(int argc, char *argv[]) {
    int in_fd;
    int count, duplicate, ignore;
    char * infile;
    count = duplicate = ignore = 0;

    if (argc <= 1) {
        usage(STD_OUT);
    } 

    if (argc > 2) {
        for (int i = 1; i < argc; i++) {
            if (strlen(argv[i]) != 2) {
                usage(STD_OUT);
            }
            switch (argv[i][1]) {
                case 'd' :
                    duplicate = 1;
                    break;
                case 'c' :
                    count = 1;
                    break;
                case 'i' :
                    ignore = 1;
                    break;
                default:
                    // assume filename
                    infile = argv[i];
            }
        }
    }

    if ((in_fd = open(infile, O_RDONLY)) < 0) {
        usage(STD_OUT);
    }

    uniq(in_fd, count, duplicate, ignore);
    exit();
    return 0;
}