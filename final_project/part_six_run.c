#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <unistd.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0

typedef struct {
    int fd;
    int block_size;
    unsigned int *buf;
    int status;
} thread_arg;

void usage() {
    printf("usage: ./run <filename> [-r|-w] <block_size> <block_count>");
}

unsigned int xorbuf(unsigned int *buffer, int size) {
    unsigned int result = 0;
    for (int i = 0; i < size; i++) {
        printf("%d\n", buffer[i]);
        result ^= buffer[i];
    }
    return result;
}

void thread_read(thread_arg *arg) {
    int r = read(arg->fd, arg->buf, arg->block_size);
    arg->status = r;
}

void disk_read(char *filename, int block_size, int num_threads) {
    int fd = open(filename, O_RDONLY);
    thread_arg *thread_args[num_threads];
    int i;
    int xor = 0;
    pthread_t threads[num_threads];
    // Initialize buffers and thread args
    for (i=0; i < num_threads; i++) {
        unsigned int buf[block_size];
        thread_arg *arg;
        arg->block_size = block_size;
        arg->fd = fd;
        arg->buf = buf;
        thread_args[i] = arg;
    }

    int loop_break = FALSE;
    while (!loop_break) {
        // pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
        for (i=0; i < num_threads; i++) {
            pthread_create(&threads[i], NULL, thread_read, (void *)thread_args[i]);
        }

        for (i=0; i < num_threads; i++) {
            // TODO - check thread_read output for r, toggle loop_break if done w file reads
            pthread_join(threads[i], NULL);
            if (thread_args[i]->status <= 0) {
                loop_break = TRUE;
            }
            xor ^= xorbuf(thread_args[i]->buf, thread_args[i]->block_size);
        }
    }
    printf("\nxor: %d\n", xor);
}

void disk_write(char *filename, int block_size, int block_count){
    int fd = open(filename, O_WRONLY|O_CREAT, 777);
    char write_letter = 't';
    char buf[block_size];

    for (int i=0; i < block_count; i++) {
        for (int j=0; j < block_size; j++) {
            buf[j] = 't';
        }
        write(fd, buf, block_size);
    }
}

int main(int argc, char *argv[]) {
    char * filename;
    char mode;
    int block_size;
    int block_count;
    int num_threads;

    if (argc != 5) {
        usage();
    }

    // TODO - Are we able to assume 
    // CLI args will come in order? @ Prof

    // pull filename

    filename = argv[1];

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

    if (mode == 'r') {
        disk_read(filename, block_size, num_threads);
    }
    else if (mode == 'w') {
        disk_write(filename, block_size, block_count);  
    }

}
