#include "types.h"
#include "stat.h"
#include "user.h"

#define STD_OUT 1
#define STD_IN 0
#define TRUE 1
#define FALSE 0
#define O_RDONLY  0x000 // should be able to include fcntl.h, but getting errors - TODO

void usage(int fd_out) {
    printf(fd_out, "usage: uniq input-file-name -w [OPTIONAL] -i [OPTIONAL] -c [OPTIONAL]\n");
    exit();
}

int string_compare(char * str1, char * str2, int ignore_case) {
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    if (len1 != len2) {
        return 0;
    }
    int i;
    char char1;
    char char2;
    for (i=0;i<len1;i++) {
        if (ignore_case && str1[i] >= 'a' && str1[i] <= 'z') {
            char1 = str1[i] - 32;
        }
        else {
            char1 = str1[i];
        }

        if (ignore_case && str2[i] >= 'a' && str2[i] <= 'z') {
            char2 = str2[i] - 32;
        }
        else {
            char2 = str2[i];
        }

        if (char1 != char2) {
            return 0;
        }

    }
    return 1;
}

int read_line(int fd_in, char * buf, int buf_size) {
    // read a 1024 byte line...
    char overflow_buf[512];
    int n;
    if (buf_size >= 512) { //pipes only hold 512 bytes!
        int total_n = 0;
        int i;
        while (total_n < buf_size) { // buf_size = 1024 always
            n = read(fd_in, overflow_buf, buf_size-total_n);
            if (n <= 0) {
                break;
            }
            total_n = total_n + n;
            for (i=0;i<n;i++) {
                buf[total_n - n + i] = overflow_buf[i]; // always add to end
            }
        }
        return total_n;
    } else {
        n = read(fd_in, buf, buf_size);
        return n;
    }
}

void write_to_pipe(int fd_in, int * pipefds) {
    // Read off lines and write them in 1024 buffer to pipe.
    int pipe_fd_write = pipefds[1];

    char read_buf[1024]; // read from fd_in
    char line_buf[1024]; // write to pipe
    int n;
    int i;
    while ((n = read(fd_in, read_buf, sizeof(read_buf))) > 0) {

        for (i=0; i<n; i++) {

            line_buf[strlen(line_buf)] = read_buf[i];

            if (read_buf[i] == '\n') {
                write(pipe_fd_write, line_buf, sizeof(line_buf));
                memset(line_buf, '\0', 1024); // reset line buf - strlen = 0 again
            }
        }
    }
    if (strlen(line_buf) > 0) {
        // TODO - only write newline if needed!
        line_buf[strlen(line_buf)] = '\n'; // add newline
        write(pipe_fd_write, line_buf, sizeof(line_buf));
    }
    close(pipe_fd_write);
}

void uniq_duplicate_mode(int * pipefds, int ignore, char * last_line) {
    // only print duplicates
    int fd_in = pipefds[0];
    int infile_num_bytes;
    int dupe_found = FALSE;
    char this_line[1024];
    while ((infile_num_bytes = read_line(fd_in, this_line, 1024)) > 0) { 
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

void uniq_non_duplicate_mode(int * pipefds, int ignore, int count, char * last_line) {
    int fd_in = pipefds[0];
    int dupe_count = 1;
    int infile_num_bytes;
    char this_line[1024];
    while ((infile_num_bytes = read_line(fd_in, this_line, 1024)) > 0) { 

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

    int pipefds[2];
    int return_status;
    int pid;
    char last_line[1024];
    return_status = pipe(pipefds);
    if (return_status == -1) {
        printf(STD_OUT, "unable to create pipe!");
        exit();
    }

    pid = fork();

    if (pid == 0) {
        // child - aka consumer
        close(pipefds[1]); // close child's write end
        int n;
        if ((n = read_line(pipefds[0], last_line, sizeof(last_line)) <= 0)) {
            printf(STD_OUT, "error - file empty");
            exit();
        }
        switch (duplicate) {
            case 0 :
                uniq_non_duplicate_mode(pipefds, ignore, count, last_line);
                break;
            case 1 :
                uniq_duplicate_mode(pipefds, ignore, last_line);
        }
        close(pipefds[0]);
        exit();
    }
    else {
        // parent process - aka pipe writer (producer)
        close(pipefds[0]); // close parent's read end
        write_to_pipe(fd_in, pipefds);
        int child_pid;
        while((child_pid = wait()) > 0); // wait for child to finish...
        exit();
    }

    return;
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