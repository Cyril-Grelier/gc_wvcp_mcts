#!/bin/bash


# Build and compile

# # release
rm -rf build_release
mkdir build_release
cd build_release || exit
# export CC=/trinity/shared/apps/cv-standard/gcc/10.1/bin/gcc
# export CXX=/trinity/shared/apps/cv-standard/gcc/10.1/bin/g++
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
cd ..

# debug
# rm -rf build
# mkdir build
# cd build || exit
# cmake -DCMAKE_BUILD_TYPE=Debug ..
# make -j
# cd ..

# # uncomment to build the docs
# cmake --build build --target docs
