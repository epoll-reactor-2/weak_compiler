#!/bin/zsh

pushd build
cmake .. -D CMAKE_BUILD_TYPE=Debug
make -j`nproc`
pushd tests
# gdb --batch -ex run -ex bt ./CodeGenTest
./CodeGenTest

popd # tests
popd # build
