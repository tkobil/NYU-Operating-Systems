#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <unistd.h>

void usage() {
    printf("usage: ./run <filename> [-r|-w] <block_size> <block_count>");
}

void disk_read(char *filename, int block_size) {
    int fd = open(filename, O_RDONLY);
    char buf[block_size];
    int r;
    while ((r=read(fd, buf, block_size)) > 0) {
        // printf("%s", buf);
        continue;
    }
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
        disk_read(filename, block_size);
    }
    else if (mode == 'w') {
        disk_write(filename, block_size, block_count);  
    }

}
