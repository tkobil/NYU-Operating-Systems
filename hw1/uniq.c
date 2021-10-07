#include "types.h"
#include "stat.h"
#include "user.h"

// Create the needed contants
#define STD_OUT 1
#define STD_IN 0
#define TRUE 1
#define FALSE 0
#define O_RDONLY  0x000 // should be able to include fcntl.h, but getting errors - TODO

void usage(int fd_out) {
// Purpose: This is just a function that will print how uniq is supposed to be used, and then exit. 
// Prerequisites: This function should only be called when the user input invalid arguments to uniq.
// Arguments: fd_out - Identifier for standard output, needed for the printf function. 
// Returns: None. 
// Side Effects: Prints a line of text, and then exits.
    printf(fd_out, "usage: uniq input-file-name -w [OPTIONAL] -i [OPTIONAL] -c [OPTIONAL]\n");
    exit();
}

int string_compare(char * str1, char * str2, int ignore_case) {
// Purpose: Determines if two strings are equal, while taking into accouunt if we should be ignoring case.  
// Prerequisites: None.
// Arguments: str1 - Pointer to the first string
//            str2 -  Pointer to the second string
//            ignore_case - True (integer of 1) if we should ignore case, false (integer of 0) otherwise 
// Returns: True (integer of 1) if the strings are considdered the same, false (integer of 0) otherwise.
// Side Effects: None.   
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    // Return false if the strings aren't the same length - to improve efficiency. 
    if (len1 != len2) {
        return 0;
    }
    int i;
    char char1;
    char char2;
    // Loop through each character in the strings, since they must now have the same length. 
    for (i=0;i<len1;i++) {
        // Convert lower case letters to upper case in str1 if the ignore flag is set
        if (ignore_case && str1[i] >= 'a' && str1[i] <= 'z') {
            char1 = str1[i] - 32;
        }
        else {
            char1 = str1[i];
        }
        // Convert lower case letters to upper case in str2 if the ignore flag is set
        if (ignore_case && str2[i] >= 'a' && str2[i] <= 'z') {
            char2 = str2[i] - 32;
        }
        else {
            char2 = str2[i];
        }
        // Check to see if the current character is the same or not;
        // jump out of the loop to return false if they are not. 
        if (char1 != char2) {
            return 0;
        }       
    }
    // At this point all characters must be the same, so return true. 
    return 1;
}

int read_line(int fd_in, char * buf, int buf_size) {
// Purpose: Read a 1024 byte line
// Prerequisites: The pipe must be open. 
// Arguments: fd_in - Integer which is the file descriptor for the input (either STDIN or an open file)
//            buf - Pointer to a 1024 byte integer array for storing the output line. 
//            buf_size - Size of the buf variable, which should always be 1024
// Returns: Number of bytes read. 
// Side Effects: Writes a line to the buf array.
    char overflow_buf[512];
    int n;
    if (buf_size >= 512) { // pipes only hold 512 bytes!
        int total_n = 0;
        int i;
        // Loop until number of bytes read equals buf_size
        while (total_n < buf_size) { // buf_size = 1024 always
            n = read(fd_in, overflow_buf, buf_size-total_n);
            // Break out of loop if we reach the end of the file
            if (n <= 0) {
                break;
            }
            total_n = total_n + n;
            // Loop through each byte read and add it to buf
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
// Purpose: Read off lines and write them in 1024 buffer to pipe.  
// Prerequisites: The pipe must be open. 
// Arguments: fd_in - Integer which is the file descriptor for the input (either STDIN or an open file)
//            pipefds - Pointer to an integer array which are the file descriptors for the pipe
// Returns: None.
// Side Effects: Prints the desired results to standard output via writing to the pipe.    
    int pipe_fd_write = pipefds[1]; // File descriptor of the write end of the pipe

    char read_buf[1024]; // read from fd_in
    char line_buf[1024]; // write to pipe
    int n;
    int i;
    // Read 1024 byte buffers from fd_in until the end of the file has been reached
    while ((n = read(fd_in, read_buf, sizeof(read_buf))) > 0) {
        // Loop through each character of the buffer that was just read.
        for (i=0; i<n; i++) {
            // Copy each character from the read buffer into the line buffer. 
            line_buf[strlen(line_buf)] = read_buf[i];
            // Write the line buffer to the pipe as soon as a newline is hit.
            if (read_buf[i] == '\n') {
                write(pipe_fd_write, line_buf, sizeof(line_buf));
                memset(line_buf, '\0', 1024); // reset line buf - strlen = 0 again
            }
        }
    }
    // If we didn't hit a newline, but there is still data in the buffer, write it to the pipe.
    if (strlen(line_buf) > 0) {
        // TODO - only write newline if needed!
        line_buf[strlen(line_buf)] = '\n'; // add newline
        write(pipe_fd_write, line_buf, sizeof(line_buf));
    }
    close(pipe_fd_write); // Close the writing end of the pipe
}

void uniq_duplicate_mode(int * pipefds, int ignore, char * last_line) {
// Purpose: Read line by line and print output in duplicate mode (only print duplicates).  
// Prerequisites: If the input is not being piped, the file must be opened already. 
// Arguments: pipefds - Pointer to an integer array which are the file descriptors for the pipe
//            ignore - Integer with a value of 1 if we should ignore case, 0 otherwise.
//            last_line - Pointer to a character array containing the last input line that was read. 
// Returns: None.
// Side Effects: Prints the desired results to standard output.   
    int fd_in = pipefds[0];
    int infile_num_bytes;
    int dupe_found = FALSE;
    char this_line[1024];
    // Read lines from input until the end of the file has been reached. 
    while ((infile_num_bytes = read_line(fd_in, this_line, 1024)) > 0) { 
        // if we find a dupe, and its the first occurence, print it and set
        // the dupe flag. Otherwise, set the dupe flag to false.
        if (string_compare(last_line, this_line, ignore) && !dupe_found) {
            printf(STD_OUT, "%s", last_line);
            dupe_found = TRUE;
        } else if (!string_compare(last_line, this_line, ignore)) {
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
// Purpose: Read line by line and print output in non-duplicate mode.  
// Prerequisites: If the input is not being piped, the file must be opened already. 
// Arguments: pipefds - Pointer to an integer array which are the file descriptors for the pipe
//            count - Integer with a value of 1 if we should prepend the number of times a line is repeated, 0 otherwise.
//            ignore - Integer with a value of 1 if we should ignore case, 0 otherwise.
//            last_line - Pointer to a character array containing the last input line that was read. 
// Returns: None.
// Side Effects: Prints the desired results to standard output.   
    int fd_in = pipefds[0];
    int dupe_count = 1;
    int infile_num_bytes;
    char this_line[1024];
    // Read lines from input until the end of the file has been reached. 
    while ((infile_num_bytes = read_line(fd_in, this_line, 1024)) > 0) { 
        // When a dupe is not found
        if (!string_compare(last_line, this_line, ignore)) {
            
            // Based on the value of the count flag, print to standard ouput 
            if (count) {
                printf(STD_OUT, "%d %s", dupe_count, last_line);
            } else {
                printf(STD_OUT, "%s", last_line);
            }

            strcpy(last_line, this_line);
            dupe_count = 1;
        // When a dupe is found, increment the dupe counter
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
// Purpose: Proxy Method for choosing to run uniq in duplicate or non-duplicate mode.  
// Prerequisites: If the input is not being piped, the file must be opened already. 
// Arguments: fd_in - Integer which is the file descriptor for the input (either STDIN or an open file)
//            count - Integer with a value of 1 if we should prepend the number of times a line is repeated, 0 otherwise.
//            duplicate - Integer with a value of 1 if we should run in duplicate mode 
//                        (only print duplicates), 0 otherwise.
//            ignore - Integer with a value of 1 if we should ignore case, 0 otherwise.
// Returns: None.
// Side Effects: Prints the desired results to standard output.    
    int pipefds[2];
    int return_status;
    int pid;
    char last_line[1024];
    // Attempt to open a pipe
    return_status = pipe(pipefds);
    // Print an error if the pipe was not successfully opened. 
    if (return_status == -1) {
        printf(STD_OUT, "unable to create pipe!");
        exit();
    }

    // Create child process
    pid = fork();

    if (pid == 0) {
        // child - aka consumer
        close(pipefds[1]); // close child's write end
        int n;
        // Attempt to read a whole line of the input, and print an error if it is empty. 
        if ((n = read_line(pipefds[0], last_line, sizeof(last_line)) <= 0)) {
            printf(STD_OUT, "error - file empty");
            exit();
        }
        // Decide which function to call (and mode to run in), based on the value of the duplicate flag. 
        switch (duplicate) {
            case 0 :
                uniq_non_duplicate_mode(pipefds, ignore, count, last_line);
                break;
            case 1 :
                uniq_duplicate_mode(pipefds, ignore, last_line);
        }
        close(pipefds[0]); // close read end
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
// Purpose: The main function to the uniq command.   
// Prerequisites: None.
// Arguments: argc - integer that designates the number of inputs that will be in the next argument. 
//            argv -  pointer to a character array that contains all of the inputs that a user gives to 
//                    uniq in the command line. 
// Returns: ineger value of 0
// Side Effects: Prints the desired results to standard output. 
    int in_fd;
    int count, duplicate, ignore;
    char * infile;
    // Initialize all flags to zero
    count = duplicate = ignore = 0;

    // Parse all arguments stored in argv to set appropriate flags and get the input filename
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

    // Check if input is file or piped input
    // from stdin, in whihc case infile will have a value of 0
    if (infile != 0) {
        in_fd = open(infile, O_RDONLY);
    }
    else {
        in_fd = STD_IN;
    }
    
    // Throw a useage message to the user if the file they provided was not able to be opened. 
    if (in_fd < 0) {
        usage(STD_OUT);
    }

    // Call uniq to do necessery tasks related to getting the correct output to the user.
    uniq(in_fd, count, duplicate, ignore);
    // Cleanup & exiting procuedure
    close(in_fd);
    exit();
    return 0;
}