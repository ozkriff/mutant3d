#!/bin/sh
make && LD_LIBRARY_PATH=../sdl2/exec/lib gdb -q mutant3d -ex "run"
