#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <unistd.h>
#include <math.h>

void usage() {
    printf("usage: ./part_one <filename> [-r|-w] <block_size> <block_count>");
}

// TODO - put in helper file
unsigned int xorbuf(unsigned int *buffer, int size) {
    unsigned int result = 0;
    for (int i = 0; i < size; i++) {
        result ^= buffer[i];
    }
    return result;
}

void disk_read(char *filename, int block_size, int block_count) {
    int fd = open(filename, O_RDONLY);
    // divide by unsigned int size because char = 1 byte, int = 4bytes
    int size = ceil(block_size/(long)sizeof(unsigned int));
    // printf("size: %d", size);
    unsigned int buf[size];
    unsigned int xor = 0;
    int r;
    int count = 0;
    unsigned int read_size;
    while (((r=read(fd, buf, size * sizeof(unsigned int))) > 0) && (count < block_count)) {
        read_size = ceil(r/sizeof(unsigned int));
        xor ^= xorbuf(buf, read_size);
        count++;
        continue;
    }
    printf("xor: %08x\n", xor);
    close(fd);
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

    if (mode == 'r') {
        disk_read(filename, block_size, block_count);
    }
    else if (mode == 'w') {
        disk_write(filename, block_size, block_count);  
    }

}
