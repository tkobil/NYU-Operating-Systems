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

int string_compare(char * str1, char * str2, int ignore_case) {
    char str_1_cmp[strlen(str1)];
    char str_2_cmp[strlen(str2)];
    if (ignore_case) {
        to_upper(str1, str_1_cmp);
        to_upper(str2, str_2_cmp);
    } else {
        strcpy(str_1_cmp, str1);
        strcpy(str_2_cmp, str2);
    }
    return (strcmp(str_1_cmp, str_2_cmp) == 0);
}

void uniq_duplicate_mode(int fd_in, int ignore, char * last_line) {
    // only print duplicates
    int infile_num_bytes;
    int dupe_found = FALSE;
    while ((infile_num_bytes = read(fd_in, this_line, sizeof(this_line))) > 0) { 
        // TODO - need to account for a line being longer than 512 bytes - readline() wrapper for read()

        // if we find a dupe, and its the first occurence, print it and set
        // the dupe flag. Otherwise, set the dupe flag to false.
        if (string_compare(last_line, this_line, ignore) && !dupe_found) {
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
    if (!string_compare(last_line, this_line, ignore) && !dupe_found) {
        printf(STD_OUT, "%s", last_line);
    }
}

void uniq_non_duplicate_mode(int fd_in, int ignore, int count, char * last_line) {
    int dupe_count = 1;
    int infile_num_bytes;
    while ((infile_num_bytes = read(fd_in, this_line, sizeof(this_line))) > 0) { 
        // TODO - need to account for a line being longer than 512 bytes - readline() wrapper for read()

        if (!string_compare(last_line, this_line, ignore)) {
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
            break;
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
        printf(STD_OUT, "there");
        usage(STD_OUT);
    } 

    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            duplicate = 1;
        } else if (strcmp(argv[i], "-c") == 0) {
            count = 1;
        } else if (strcmp(argv[i], "-i") == 0) {
            ignore = 1;
        } else {
            infile = argv[i];
        }
    }

    in_fd = open(infile, O_RDONLY);
    if (in_fd < 0) {
        printf(STD_OUT, "%d", in_fd);
        printf(STD_OUT, "%s", infile);
        usage(STD_OUT);
    }

    uniq(in_fd, count, duplicate, ignore);
    exit();
    return 0;
}