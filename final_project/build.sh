#!/usr/bin/env bash

gcc -pthread run_multithreaded.c -o run_multithreaded 
gcc run.c -o run
gcc run_blocksize.c -o run_blocksize
gcc fast.c -o fast