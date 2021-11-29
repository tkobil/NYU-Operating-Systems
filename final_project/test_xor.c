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
    unsigned int buf[128];
    int i;
    int size;
    unsigned int xor = 0;
    unsigned int answ;
    int fd = open(filename, O_RDONLY);
    while ((i = read(fd, buf, 128 * sizeof(unsigned int))) > 0) {
        size = i/sizeof(unsigned int);
        xor ^= xorbuf(buf, size);
    }
    printf("%08x\n", xor);
    close(fd);
}