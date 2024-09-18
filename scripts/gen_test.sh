#!/usr/bin/bash
make clean
make
pushd build
LD_LIBRARY_PATH=lib ./bin/gen_test
chmod +x   ./outputs/gen/__gen.o
echo ""
echo " - Running generated ELF file..."
qemu-riscv64 ./outputs/gen/__gen.o
echo " - Returned with $?"
echo ""
popd # build