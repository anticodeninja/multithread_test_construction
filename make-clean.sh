#!/bin/bash

ORIG=`pwd`
cd `dirname $0`
rm CMakeCache.txt
rm -rf CMakeFiles
cd $ORIG
