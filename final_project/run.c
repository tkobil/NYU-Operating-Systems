#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <pthread.h>
#include <unistd.h>

typedef struct {
    int fd;
    int size;
    unsigned int *buf;
} thread_arg;

void usage() {
    printf("usage: ./run <filename> [-r|-w] <block_size> <block_count> <num_threads>");
}

unsigned int xorbuf(unsigned int *buffer, int size) {
    unsigned int result = 0;
    for (int i = 0; i < size; i++) {
        result ^= buffer[i];
    }
    return result;
}

void *safe_read(void *arg) {
    thread_arg *arg_struct = (thread_arg *)arg;
    read(arg_struct->fd, arg_struct->buf, arg_struct->size);
    pthread_exit(NULL);
}

void disk_read(char *filename, int block_size, int block_count, int num_threads) {
    // Read From Disk, XOR all 4-byte integers of file and print.
    int fd = open(filename, O_RDONLY);

    unsigned int *buffers[num_threads];
    
    // We want each thread to handle a portion of a buffer
    if (num_threads > block_size) {
        printf("Error, more threads than block size!");
    }

    int size_per_thread = block_size / num_threads;
    pthread_t threads[num_threads];

    // Create threads
    for (int i=0; i < num_threads; i++) {
        int size = size_per_thread;

        if (i == num_threads - 1) {
            size += block_size % num_threads;
        }

        // Initialize thread args
        thread_arg *arg;
        unsigned int buf[size];
        arg->fd = fd;
        arg->size = size;
        arg->buf = buf;

        pthread_create(&threads[i], NULL, safe_read, (void *)arg);

    }

    // Join threads
    for (int i=0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    unsigned int xor;
    for (int i=0; i < num_threads; i++) {
        if (i==0) {
            xor = xorbuf(buffers[i], size_per_thread);
        } else if (i == num_threads - 1) {
            xor ^= xorbuf(buffers[i], num_threads + (block_size % num_threads));
        }
        else {
            xor ^= xorbuf(buffers[i], size_per_thread);
        }
    }

    printf("%08x\n", xor);
}

void disk_write(char *filename, int block_size, int block_count, int num_threads) {
    // Write to Disk - TODO
}

int main(int argc, char *argv[]) {
    char * filename;
    char mode;
    int block_size;
    int block_count;
    int num_threads; // TODO - delete num_threads and optimize

    if (argc != 6) {
        usage();
    }

    // TODO - Are we able to assume 
    // CLI args will come in order? @ Prof

    // pull filename
    filename = argv[1];
    // strcpy(filename, argv[1]);

    // get read/write flag
    if (strlen((argv[2])) != 2) {
        usage();
    }
    else {
        mode = argv[2][1];
    }

    block_size = atoi(argv[3]);
    block_count = atoi(argv[4]);
    num_threads = atoi(argv[5]);

}




// Questions:
// 1. When in read mode, we specify block size and block num. What happens when 
// these don't add up to equal total size of file?
// 2. What do we write in write mode?
// 3. Will CLI args always come in the same order? i.e. there are no flags