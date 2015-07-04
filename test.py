#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import subprocess
import shutil
import sys


BIN_DIR = "bin"
BIN_NAME = "multithread"
CLEAN_LIST = ["CMakeCache.txt", "cmake_install.cmake", "Makefile", "CMakeFiles"]
RESULT_DIR = "result"
REDUCE_BIN = "scripts/reduce_time_data.py"
TEST_DIR = "test"
TEST_SET = [
    dict(name = "StSm", flags = ["TIME_PROFILE"]),
    dict(name = "StMm", flags = ["TIME_PROFILE", "DIFFERENT_MATRICES"]),
    dict(name = "MtSm", flags = ["TIME_PROFILE", "MULTITHREAD"]),
    dict(name = "MtMm", flags = ["TIME_PROFILE", "MULTITHREAD", "DIFFERENT_MATRICES"])
]

current_dir = os.getcwd()
cmake_path = os.path.join(current_dir, "CMakeLists.txt")
reduce_path = os.path.join(current_dir, REDUCE_BIN)

print("CMakeProjectPath: %s" % cmake_path)

DATA_SIZE = 11
if len(sys.argv) > 1:
    DATA_SIZE = sys.argv[1]
DATA_SET = "data/input_data_%s.txt" % DATA_SIZE

print("Input data set: %s" % DATA_SET)

for test in TEST_SET:
    print("Test: %s" % test['name'])
    os.chdir(current_dir)
    subprocess.check_call(["rm", "-rf"] + CLEAN_LIST)
    subprocess.check_call(["cmake", cmake_path] + ["-D%s=ON" % flag for flag in test['flags']])
    subprocess.check_call("make")

    test_path = os.path.join(current_dir, TEST_DIR, test['name'])
    print("Test dir %s in %s" % (test['name'], test_path))
    os.makedirs(test_path, exist_ok = True)
    os.chdir(test_path)
    shutil.copy(os.path.join(current_dir, BIN_DIR, BIN_NAME), BIN_NAME)
    shutil.copy(os.path.join(current_dir, DATA_SET), "input_data.txt")

    print("Run %s in %s" % (test['name'], test_path))
    subprocess.check_call(os.path.join(".", BIN_NAME))

    result_path = os.path.join(current_dir, RESULT_DIR, test['name'])
    print("Collect timelogs for %s in %s" % (test['name'], result_path))
    os.makedirs(result_path, exist_ok = True)
    os.chdir(result_path)

    shutil.copy(os.path.join(test_path, "time_collector.txt"), ".")
    subprocess.check_call([reduce_path])

