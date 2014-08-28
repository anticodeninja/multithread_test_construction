#!/usr/bin/env python
# -*- coding: utf-8 -*-

import unittest
import utils

__author__ = 'zzloiz'

class RangesTest(unittest.TestCase):

    def test_interference(self):
        main_range = [5, 10]

        self.assertFalse(utils.intersect_range(main_range, [1, 3]))
        self.assertEqual(main_range, [5, 10])

        self.assertFalse(utils.intersect_range(main_range, [11, 13]))
        self.assertEqual(main_range, [5, 10])

        self.assertTrue(utils.intersect_range(main_range, [6, 7]))
        self.assertEqual(main_range, [5, 10])

        self.assertTrue(utils.intersect_range(main_range, [3, 5]))
        self.assertEqual(main_range, [3, 10])

        self.assertTrue(utils.intersect_range(main_range, [10, 13]))
        self.assertEqual(main_range, [3, 13])

        self.assertTrue(utils.intersect_range(main_range, [3, 16]))
        self.assertEqual(main_range, [3, 16])

        self.assertTrue(utils.intersect_range(main_range, [2, 16]))
        self.assertEqual(main_range, [2, 16])

        self.assertTrue(utils.intersect_range(main_range, [2, 16]))
        self.assertEqual(main_range, [2, 16])

        self.assertTrue(utils.intersect_range(main_range, [1, 17]))
        self.assertEqual(main_range, [1, 17])

    def test_combine_ranges(self):
        main_ranges = [[1,3], [5,6]]

        utils.combine_ranges(main_ranges, [9,10])
        self.assertEqual(main_ranges, [[1,3], [5,6], [9,10]])

        utils.combine_ranges(main_ranges, [7,8])
        self.assertEqual(main_ranges, [[1,3], [5,6], [7,8], [9,10]])

        utils.combine_ranges(main_ranges, [2,4])
        self.assertEqual(main_ranges, [[1,4], [5,6], [7,8], [9,10]])

        utils.combine_ranges(main_ranges, [8,9])
        self.assertEqual(main_ranges, [[1,4], [5,6], [7,10]])

        utils.combine_ranges(main_ranges, [11,13])
        self.assertEqual(main_ranges, [[1,4], [5,6], [7,10], [11,13]])

        utils.combine_ranges(main_ranges, [2,9])
        self.assertEqual(main_ranges, [[1,10], [11,13]])

        utils.combine_ranges(main_ranges, [10,11])
        self.assertEqual(main_ranges, [[1,13]])