#!/bin/bash

set -e

gcc -c level.c -o level.o
gcc -c AI_list.c -o AI_list.o 


if [[ "$1" == "main" ]] || [[ "$1" == "all" ]]; then
	gcc main.c -o super-hydric-guy level.o AI_list.o -lraylib -lX11 -lm
fi

if [[ "$1" == "level" ]] || [[ "$1" == "all" ]]; then
	gcc level_edit.c raygui_impl.c -o editor level.o AI_list.o -lraylib -lX11 -lm
fi

if [[ "$1" == "ai" ]] || [[ "$1" == "all" ]]; then
	gcc AI_edit.c raygui_impl.c -o AI_editor AI_list.o -lraylib -lX11 -lm
fi

if [[ "$1" != "main" ]] && [[ "$1" != "level" ]] && [[ "$1" != "ai" ]] && [[ "$1" != "all" ]]; then
	echo "Invalid argument '$1'"
	echo "Usage:"
	echo "./build.sh main    -  builds the game and other components"
	echo "./build.sh level   -  builds the level editor and other components"
	echo "./build.sh ai      -  builds the AI editor and other components"
	echo "./build.sh all     -  builds everything"
else
	echo "Compiled successfully."
fi
