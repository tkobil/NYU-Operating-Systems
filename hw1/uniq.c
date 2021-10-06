#include "types.h"
#include "stat.h"
#include "user.h"

#define STD_OUT 1
#define STD_IN 0
#define TRUE 1
#define FALSE 0
#define O_RDONLY  0x000 // should be able to include fcntl.h, but getting errors - TODO

//hardcoded for now
char last_line[1024];
char this_line[1024];
char one_bit[1];

void usage(int fd_out) {
    printf(fd_out, "usage: uniq input-file-name -w [OPTIONAL] -i [OPTIONAL] -c [OPTIONAL]");
    exit();
}

// Purpose: This function reads one line of a file per call, one byte at a time. 
// Prerequisites: File must already be opened. 
// Arguments:
//       in_fd - identifier of opened file
//       one_bit - a pointer to an empty, one byte character array 
//       line - a pointer to an empty, 1024 byte character array 
// Returns: Number of bytes last read in. Will either be 1 or 0.
// Side effects: fills in the array provided in the "line" argument with the first line of the opened file. 
int myreadline(int in_fd, char * one_bit, char * line){
    int position = 0;
    int endofLine = FALSE;
    int num_bytes;
    // Have to clear the data in this line in order to avoid data leaks when loading the next one. 
    memset(line,0,1024);
    // While the newline character has not yet been reached
    while (!endofLine) {
        // Read one byte from the file and store it in one_bit
        num_bytes = read(in_fd, one_bit, 1);
        // If the current byte is the new line character, or if we hit the end of the file, jump out of the loop
        if ((strcmp(one_bit, "\n") == 0) || num_bytes == 0) {
            line[position] = '\n';
            endofLine = TRUE;
        // Otherwise, store the byte in the next spot in the "line" array.     
        } else {
            //if (position > 512) {
                 // right now we just have two 1024 byte arrays allocated, but  we might 
                 // want to use a smaller array and then reallocate the size of the array (to be more effient
                 // for most cases when lines are short). That reallocation could be done here. 
            //}
            line[position] = one_bit[0];
            position++;
        }
    }
    if (num_bytes == 0 && position == 0){
        num_bytes = 0;
    } else {
        num_bytes = 1;
    }
    return num_bytes;    
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
    while ((infile_num_bytes = myreadline(fd_in, one_bit, this_line)) > 0) { 
        
        // if we find a dupe, and its the first occurence, print it and set
        // the dupe flag. Otherwise, set the dupe flag to false.
        if (string_compare(last_line, this_line, ignore) && !dupe_found) {
            printf(STD_OUT, "%s", last_line);
            dupe_found = TRUE;
        } else {
            memset(last_line,0,1024);
            strcpy(last_line, this_line);

            dupe_found = FALSE;
        }
    }
}

void uniq_non_duplicate_mode(int fd_in, int ignore, int count, char * last_line) {
    int dupe_count = 1;
    int infile_num_bytes;
    //read(fd_in, last_line, sizeof(last_line))
    while ((infile_num_bytes = myreadline(fd_in, one_bit, this_line)) > 0) { 
        if (!string_compare(last_line, this_line, ignore)) {
            // dupe not found
            if (count) {
                printf(STD_OUT, "%d %s", dupe_count, last_line);
            } else {
                printf(STD_OUT, "%s", last_line);
            }
            memset(last_line,0,1024);
            strcpy(last_line, this_line);
            dupe_count = 1;
        } else {
            dupe_count++;
        }
        // Have to clear the data in this line in order to avoid data leaks when loading the next one. 
        memset(this_line,0,1024);

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
    if ((n = myreadline(fd_in, one_bit, last_line)) <= 0) {
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

    // check if input is file or piped input
    // from stdin
    if (infile != 0) {
        in_fd = open(infile, O_RDONLY);
    }
    else {
        in_fd = STD_IN;
    }

    if (in_fd < 0) {
        usage(STD_OUT);
    }

    uniq(in_fd, count, duplicate, ignore);
    close(in_fd);
    exit();
    return 0;
}
