#!/usr/bin/bash
make clean
make
pushd build
LD_LIBRARY_PATH=lib ./bin/elf_test
objdump -D ./outputs/elf/__elf.o
chmod +x   ./outputs/elf/__elf.o
echo ""
echo " - Running generated ELF file..."
LD_VERBOSE=1 ./outputs/elf/__elf.o
echo " - Returned with $?"
echo ""
popd # build