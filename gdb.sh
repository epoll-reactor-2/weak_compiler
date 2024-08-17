#!/bin/zsh

pushd build
pushd tests
gdb ./CodeGenTest
popd # tests
popd # build