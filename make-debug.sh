#!/bin/bash

ORIG=`pwd`
cd `dirname $0`
cmake CMakeLists.txt -DDEBUG_MODE=ON -DDIFFERENT_MATRICES=ON -DCMAKE_BUILD_TYPE=Debug
make
cd $ORIG
./multithread
