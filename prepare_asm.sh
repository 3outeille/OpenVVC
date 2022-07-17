#!/bin/bash

cd examples
rm fg_compute_block_avg_asm.o
nasm -g -f elf64 fg_compute_block_avg_asm.asm
cd ..
ls examples
