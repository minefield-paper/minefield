#!/bin/bash

DIR=`dirname "$0"`
$DIR/llvm/build/bin/$1 ${@:2}
#$DIR/llvm/build_debug/bin/$1 ${@:2}

# -mllvm -fh-mask
# usefull flags  -mllvm -print-after-all 
# -mllvm -print-after-all -mllvm -print-before-all -ggdb3 -emit-llvm  -mllvm -print-after-all

