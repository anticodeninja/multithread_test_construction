#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import subprocess
import shutil

BIN_DIR = "bin"
RESULT_DIR = "result"
DATA_SET = "data/input_data_6.txt"
REDUCE_BIN = "scripts/reduce_time_data.py"
TEST_SET = [
    dict(name = "StSm", flags = ["TIME_PROFILE"]),
    dict(name = "StMm", flags = ["TIME_PROFILE", "DIFFERENT_MATRICES"]),
    dict(name = "MtSm", flags = ["TIME_PROFILE", "MULTITHREAD"]),
    dict(name = "MtMm", flags = ["TIME_PROFILE", "MULTITHREAD", "DIFFERENT_MATRICES"])
]

current_dir = os.getcwd()
qmake_path = os.path.join(current_dir, "multithread", "multithread.pro")
reduce_path = os.path.join(current_dir, REDUCE_BIN)

print("QMakeProjectPath: %s" % qmake_path)

for test in TEST_SET:
    build_path = os.path.join(current_dir, BIN_DIR, test['name'])
    print("Build %s in %s" % (test['name'], build_path))
    os.makedirs(build_path, exist_ok = True)
    os.chdir(build_path)

    args = ["DEFINES+=%s" % flag for flag in test['flags']]
    subprocess.check_call(["qmake", os.path.relpath(qmake_path, build_path)] + args)
    subprocess.check_call("make")
    shutil.copy(os.path.join(current_dir, DATA_SET), "input_data.txt")
    subprocess.check_call("./multithread")

    result_path = os.path.join(current_dir, RESULT_DIR, test['name'])
    print("Collect timelogs for %s in %s" % (test['name'], result_path))
    os.makedirs(result_path, exist_ok = True)
    os.chdir(result_path)

    shutil.copy(os.path.join(build_path, "time_collector.txt"), "./")
    subprocess.check_call([reduce_path])


