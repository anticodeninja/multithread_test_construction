#!/bin/bash

ORIG=`pwd`
cd `dirname $0`
cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=Debug -DDEBUG_MODE=ON -DTIME_PROFILE=ON
# -DDEBUG_MODE=ON -DDIFFERENT_MATRICES=ON -DMULTITHREAD=ON
make
cd $ORIG
./multithread
cat <<PYTHON_SCRIPT | python
import sys
sys.path.append("`dirname $0`/scripts")
import utils
utils.validate_result("reference.txt", "output_data.txt")
PYTHON_SCRIPT
