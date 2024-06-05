#!/bin/bash 

cmake --build build                         
./build/autoparallelizer tests/sums.c
