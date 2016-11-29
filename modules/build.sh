#!/bin/bash

echo "build common..."
echo "========================"

cd build
rm -fr *
cmake -DCMAKE_CXX_COMPILER=/usr/local/bin/g++ -DCMAKE_C_COMPILER=/usr/local/bin/gcc ..
make -j4

echo "build access_manager..."
echo "========================"

cd ..
cd module/access_manager/build
rm -fr *

cmake -DCMAKE_CXX_COMPILER=/usr/local/bin/g++ -DCMAKE_C_COMPILER=/usr/local/bin/gcc ..
make -j4

echo "build completed."
echo "========================"
