#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <unistd.h>
#include <benchmark/benchmark.h>

void disk_read(char *filename, int block_size) {
    int fd = open(filename, O_RDONLY);
    char buf[block_size];
    int r;
    //int count = 0;
    while ((r=read(fd, buf, block_size)) > 0) {
        //count += 1;
        continue;
    }
    //printf("%d", count);
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


static void BM_Write(benchmark::State& state) {
  char * filename;
  int block_size;
  int block_count;
  filename = (char*)"benchmarktest.txt";
  block_size = 512;
  block_count = 1953125;
  for (auto _ : state)
    disk_write(filename, block_size, block_count);
}
// Register the function as a benchmark
BENCHMARK(BM_Write);

// Define another benchmark
static void BM_Read(benchmark::State& state) {
  char * filename;
  int block_size;
  filename = (char*)"benchmarktest.txt";
  //int block_count;
  block_size = 512;
  //block_count = 195;
  for (auto _ : state)
    disk_read(filename, block_size);
}
BENCHMARK(BM_Read);

BENCHMARK_MAIN();
