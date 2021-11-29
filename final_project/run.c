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
    unsigned int xor;
    int finished;
    int thread_num;
    int num_threads;
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

void *thread_read(void *arg) {
    thread_arg *t_arg = (thread_arg *)arg;
    int r;
    int size;
    int offset = t_arg->thread_num * t_arg->block_size;
    int count = 0;
    while ((r = pread(t_arg->fd, t_arg->buf, t_arg->block_size, offset)) > 0) {
        size = r/sizeof(unsigned int);
        t_arg->xor ^= xorbuf(t_arg->buf, size);
        count++;
        offset = ((t_arg->num_threads * count) + t_arg->thread_num) * t_arg->block_size;
    }
    // int r = read(t_arg->fd, t_arg->buf, t_arg->block_size);
    t_arg->finished = TRUE;
    pthread_exit(NULL);
}

void disk_read(char *filename, int block_size, int num_threads) {
    int fd = open(filename, O_RDONLY);
    thread_arg thread_args[num_threads];
    int i;
    int xor = 0;
    pthread_t threads[num_threads];
    unsigned int buf_size = block_size / sizeof(unsigned int);
    //Initialize buffers and thread args
    for (i=0; i < num_threads; i++) {
        thread_arg arg;
        arg.block_size = buf_size * sizeof(unsigned int);
        arg.fd = fd;
        arg.thread_num = i;
        arg.num_threads = num_threads;
        arg.buf = (unsigned int *)malloc(buf_size * sizeof(unsigned int));
        arg.xor = 0;
        arg.finished = FALSE;
        thread_args[i] = arg;
    }

    for (i=0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, thread_read, (void *)(&thread_args[i]));
    }

    // Wait till at least one thread finishes. As soon as it does,
    // we know file has been read...
    int loop_break = FALSE;
    while(!loop_break) {
        for (i=0; i < num_threads; i++) {
            if (thread_args[i].finished) {
                loop_break = TRUE;
            }
        }
    }

    for (i=0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        printf("thread %d\n", i);
        for (int z=0; z < sizeof(thread_args[i].buf); z++) {
            printf("%d\n", thread_args[i].buf[z]);
        }
        // printf("%d\n", thread_args[i]->buf);
        xor ^= thread_args[i].xor;
    }

    printf("xor: %08x\n", xor);
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
    if (argc != 6) {
        usage();
    }

    // TODO - Are we able to assume 
    // CLI args will come in order? @ Prof
    filename = argv[1];

    // get read/write flag
    if (strlen(argv[2]) != 2) {
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
