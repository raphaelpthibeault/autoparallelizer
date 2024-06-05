#!/bin/bash 

cmake --build build                         
./build/autoparallelizer tests/fft.c
