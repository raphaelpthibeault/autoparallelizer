#!/bin/bash 

cmake --build build                         
./build/autoparallelizer tests/hello_world.c
