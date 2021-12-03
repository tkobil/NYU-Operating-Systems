#!/usr/bin/env bash

gcc -pthread run.c -o run 
gcc part_one_run.c -o part_one
gcc run_final.c -o run_final
gcc fast.c -o fast