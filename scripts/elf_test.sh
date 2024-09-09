#!/usr/bin/bash
make clean
make
pushd build
LD_LIBRARY_PATH=lib ./bin/elf_test
chmod +x ./outputs/elf/__elf.o
LD_VERBOSE=1 ./outputs/elf/__elf.o
popd # build