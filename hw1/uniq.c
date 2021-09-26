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
    printf(fd_out, "usage: uniq input-file-name -w [OPTIONAL] -i [OPTIONAL] -c [OPTIONAL]");
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

void uniq(int fd_in, int count, int duplicate, int ignore) {
    int dupe_count = 1;
    int infile_num_bytes, n;
    if ((n = read(fd_in, last_line, sizeof(last_line)) <= 0)) {
        printf(STD_OUT, "error - file empty");
        exit();
    }

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

int main(int argc, char *argv[]) {
    int in_fd;
    int count, duplicate, ignore;
    count = duplicate = ignore = 0;

    if (argc <=1) {
        usage(STD_OUT);
        exit();
    }
    else {
        // argv[0] is uniq, argv[1] is input file name
        in_fd = open(argv[1], O_RDONLY);

        if (argc > 2) {
            for (int i = 2; i < argc; i++) {
                if (strlen(argv[i]) != 2) {
                    usage(STD_OUT);
                    exit();
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
                        usage(STD_OUT);
                        exit();
                }

            }
        }
    }
    uniq(in_fd, count, duplicate, ignore);
    exit();
    return 0;
}