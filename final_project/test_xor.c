#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <unistd.h>


// TODO - put in helper file
unsigned int xorbuf(unsigned int *buffer, int size) {
    unsigned int result = 0;
    for (int i = 0; i < size; i++) {
        result ^= buffer[i];
    }
    return result;
}


int main(int argc, char *argv[]) {
    
    char *filename = argv[1];
    unsigned int buf[512];
    int i;
    unsigned int xor;
    int fd = open(filename, O_RDONLY);
    while ((i = read(fd, buf, 512)) > 0) {
        // printf("%d\n", i);
        xor ^= xorbuf(buf, i);
    }
    printf("%08x\n", xor);
    close(fd);
}