#!/bin/bash 

cmake --build build                         
./build/autoparallelizer tests/foo.c
