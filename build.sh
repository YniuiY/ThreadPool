#!/bin/bash
DIR=`pwd`
if [ -d build ]; then
    rm -rf build
fi
mkdir build

IS_ARM=false
IS_ARM6=false
IS_ARM9=false
IS_ARM10=false
IS_ARM12=false
IS_INSTALL=false

for i in "$@"; do
  if [ "$i" = "-platform=arm6" -o "$i" = "-a6" ]; then
    IS_ARM6=true
  elif [ "$i" = "-platform=arm9" -o "$i" = "-a9" ]; then
    IS_ARM9=true
  elif [ "$i" = "-platform=arm10" -o "$i" = "-a10" ]; then
    IS_ARM10=true
  elif [ "$i" = "-platform=arm12" -o "$i" = "-a12" ]; then
    IS_ARM12=true
  elif [ "$i" = "-install=true" -o "$i" = "-i" ]; then
    IS_INSTALL=true
  fi
done

# Support the arm compile, if the arm plaform need to be supported, please
# enable the CMAKE_COMMAND
if [ $IS_ARM6 = true ]; then
  CMAKE_COMMAND=-DCOMPILE_TOOLCHAIN_ROOT=$DIR/../autodrive-arm
elif [ $IS_ARM9 = true ]; then
  CMAKE_COMMAND=-DCOMPILE_TOOLCHAIN_ROOT_9_3_0=$DIR/../autodrive-arm
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/../autodrive-arm/gcc-ubuntu-9.3.0-2020.03-x86_64-aarch64-linux-gnu/lib/x86_64-linux-gnu
elif [ $IS_ARM10 = true ]; then
  CMAKE_COMMAND=-DCOMPILE_TOOLCHAIN_ROOT_10_3_1=$DIR/../autodrive-arm
elif [ $IS_ARM12 = true ]; then
  CMAKE_COMMAND=-DCOMPILE_TOOLCHAIN_ROOT_12_1_1=$DIR/../autodrive-arm
fi

cd build
cmake .. $CMAKE_COMMAND
make -j16
make install
cd ..

cd sample
sh build.sh $CMAKE_COMMAND
cd ..
