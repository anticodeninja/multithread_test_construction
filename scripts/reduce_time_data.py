#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import utils

__author__ = 'zzloiz'

states = {}

input_file = open("time_collector.txt", encoding='utf-8')
data_count = int(input_file.readline())

data_skip = int(data_count / 100)
if data_skip == 0:
    data_skip = 1
data_current = 0

for line in input_file:
    if data_current % data_skip == 0:
        print("%d%%" % int(data_current/data_skip))
    data_current += 1

    data = line.split(' ')
    thread, task, time_start, time_stop = int(data[0]), str(data[1]), int(data[2]), int(data[3])

    if task not in states:
        states[task] = dict(ranges=[], sum=0, count=0)

    utils.combine_ranges(states[task]['ranges'], [time_start, time_stop])
    states[task]['sum'] += time_stop - time_start
    states[task]['count'] += 1

info_file = open("time_collector_info.txt", "w", encoding='utf-8')
info=dict(
    sum = {k : states[k]['sum'] for k in states},
    count = {k : states[k]['count'] for k in states},
    reduced_sum = {k : sum(d[1] - d[0] for d in states[k]['ranges']) for k in states},
    reduced_count = {k : len(states[k]) for k in states}
)

reduced_file = open("time_collector_reduced.txt", "w", encoding='utf-8')
states = {k : states[k]['ranges'] for k in states}
json.dump(states, reduced_file)
reduced_file.close()

json.dump(info, info_file)
info_file.close()

