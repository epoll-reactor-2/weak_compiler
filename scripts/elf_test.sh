#!/usr/bin/bash
make clean
make
pushd build
LD_LIBRARY_PATH=lib ./bin/elf_test
riscv64-linux-gnu-objdump -D ./outputs/elf/__elf.o
chmod +x   ./outputs/elf/__elf.o
echo ""
echo " - Running generated ELF file..."
qemu-riscv64 ./outputs/elf/__elf.o
echo " - Returned with $?"
echo ""
popd # build