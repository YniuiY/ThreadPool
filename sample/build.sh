#!/bin/bash
DIR=`pwd`
rm -rf build
mkdir build

cd build
cmake .. $1
make -j4
cd $DIR