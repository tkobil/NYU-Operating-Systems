#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <unistd.h>
#include <math.h>

void usage() {
    printf("usage: ./part_one <filename>");
}

// TODO - put in helper file
unsigned int xorbuf(unsigned int *buffer, int size) {
    unsigned int result = 0;
    for (int i = 0; i < size; i++) {
        result ^= buffer[i];
    }
    return result;
}

void disk_read(char *filename) {
    int block_size = 4096;
    int fd = open(filename, O_RDONLY);
    int size = ceil(block_size/(long)sizeof(unsigned int));
    unsigned int buf[size];
    unsigned int xor = 0;
    int r;
    unsigned int read_size;

    while ((r=read(fd, buf, size * sizeof(unsigned int))) > 0) {

        read_size = ceil(r/sizeof(unsigned int));
        xor ^= xorbuf(buf, read_size);
    }
    printf("xor: %08x\n", xor);
    close(fd);
}


int main(int argc, char *argv[]) {
    char * filename;
    if (argc != 2) {
        usage();
    }
    filename = argv[1];

    disk_read(filename);
}
