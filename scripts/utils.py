#!/usr/bin/env python
# -*- coding: utf-8 -*-

__author__ = 'zzloiz'

import os
import ipdb

def enumerate_file(filename):
    for line in open(filename, "r"):
        for word in line.split():
            yield int(word)

def intersect_range(range1, range2):
    if range2[0] > range1[0] and range2[1] < range1[1]:
        return True
    if range2[0] < range1[0] and range2[1] < range1[0]:
        return False
    if range2[0] > range1[1] and range2[1] > range1[1]:
        return False

    if range2[0] < range1[0] and range2[1] > range1[1]:
        range1[0] = range2[0]
        range1[1] = range2[1]
        return True
    if range2[0] < range1[0] and range2[1] >= range1[0]:
        range1[0] = range2[0]
        return True
    if range2[0] <= range1[1] and range2[1] > range1[1]:
        range1[1] = range2[1]
        return True

    return True

def combine_ranges(ranges, range1):
    i = len(ranges)
    while i >= 0:
        prev = ranges[i-1] if i>0 else None
        next = ranges[i] if i<len(ranges) else None

        if (prev == None or range1[0] > prev[1]) and (next == None or range1[1] < next[0]):
            ranges.insert(i, range1)
            return

        j = i-1
        while intersect_range(ranges[j], range1) and j>=0:
            range1 = ranges[j]
            j -= 1

        if j != i-1:
            del ranges[j+2:i]
            return

        i-=1
        
def validate_result(reference_file, result_file):
    if not os.path.exists(reference_file):
        return
    
    reference = enumerate_file(reference_file)
    result = enumerate_file(result_file)

    rows = next(reference)
    cols = next(reference)

    if rows != next(result):
        raise Exception("Incorrect rows count")

    if cols != next(result):
        raise Exception("Incorrect cols count")

    try:
        read_matrix = lambda x: set(tuple(next(x) for j in range(cols)) for i in range(rows))
        reference_data = read_matrix(reference)
        result_data = read_matrix(result)
    except:
        raise Exception("Cannot parse output")

    if reference_data != result_data:
        print("reference additionals:")
        for i in (reference_data - result_data):
            print(i)
        print("result additionals")
        for i in (result_data - reference_data):
            print(i)
        raise Exception("Non-identical output")

    try:
        read_p = lambda x: [next(x) for i in range(cols)]
        reference_p = read_p(reference)
        result_p = read_p(result)
    except:
        raise Exception("Cannot parse p")

    if reference_p != result_p:
        raise Exception("Non-identical p")

def compact_time_metric(profile_path, metric_path=None, timeline_path=None):
    states = {}

    profile_file = open(profile_path, encoding='utf-8')
    data_count = int(input_file.readline())

    data_skip = int(data_count / 100)
    if data_skip == 0:
        data_skip = 1
    data_current = 0

    for line in profile_file:
        if data_current % data_skip == 0:
            print("%d%%" % int(data_current/data_skip), end='\r')
            
        data_current += 1

        data = line.split(' ')
        thread, task, time_start, time_stop = int(data[0]), str(data[1]), int(data[2]), int(data[3])

        if task not in states:
            states[task] = dict(ranges=[], sum=0, count=0)

        utils.combine_ranges(states[task]['ranges'], [time_start, time_stop])
        states[task]['sum'] += time_stop - time_start
        states[task]['count'] += 1

    if not metric_path:
        metric_file = open(metric_path, "w", encoding='utf-8')
        metric = dict(
            sum = {k : states[k]['sum'] for k in states},
            count = {k : states[k]['count'] for k in states},
            reduced_sum = {k : sum(d[1] - d[0] for d in states[k]['ranges']) for k in states},
            reduced_count = {k : len(states[k]) for k in states}
        )
        json.dump(info, metric_file)
        metric_file.close()

    if not timeline_path:
        timeline_file = open(timeline_path, "w", encoding='utf-8')
        states = {k : states[k]['ranges'] for k in states}
        json.dump(states, timeline_file)
        timeline_file.close()

