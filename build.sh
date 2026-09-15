#!/bin/bash

set -e

gcc -c level.c -o level.o
gcc main.c -o super-hydric-guy level.o -lraylib -lX11 -lm
gcc level_edit.c -o editor level.o -lraylib -lX11 -lm