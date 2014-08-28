#!/usr/bin/env python
# -*- coding: utf-8 -*-

__author__ = 'zzloiz'

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
