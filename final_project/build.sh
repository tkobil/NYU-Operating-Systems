#!/usr/bin/env bash

gcc -pthread run.c -o run 
gcc -pthread run_multithreaded.c -o run_multithreaded 
gcc part_one_run.c -o part_one
