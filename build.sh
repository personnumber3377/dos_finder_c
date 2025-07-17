#!/bin/sh

clang mutator.c -fsanitize=address,undefined -DTESTING=1 -o main_test
clang mutator.c -O3 -shared -o libmutator.so

