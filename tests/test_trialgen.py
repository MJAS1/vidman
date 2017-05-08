#!/usr/bin/env python2
# -*- coding: utf-8 -*-
"""
Created on Fri May  5 17:30:58 2017

@author: sutelam1
"""
import sys
import unittest

sys.path.append('../trials')
import trialgen as tg

class TestTrialgen(unittest.TestCase):
    """Tests that trialgen.py is producing trials correctly.
    """

    def test_trig_count(self):
        out = tg.create_trial()
        standards = out.count("trigcode={}".format(tg.TRIG_STANDARD))
        deviants = out.count("trigcode={}".format(tg.TRIG_DEVIANT))
        blanks = out.count("trigcode={}".format(tg.TRIG_BLANK))

        self.assertEqual(standards, 900+20)
        self.assertEqual(deviants, 100)
        self.assertEqual(blanks, 250)


    def test_distance(self):
        for i in range(0, 100):
            out = tg.create_trial()
            lines = out.splitlines()
            distances = []
            dist = 0
            first = True
            for line in lines:
                if "trigcode={}".format(tg.TRIG_DEVIANT) in line:
                    if first == False:
                        distances.append(dist)
                        dist = 0
                    else:
                        first = False
                elif "trigcode={}".format(tg.TRIG_STANDARD) in line:
                    dist += 1
                elif "Pause" in line:
                    first = True
                    dist = 0
            
            self.assertEqual(all(d >= 5 for d in distances), True)


    def test_pause_count(self):
        out = tg.create_trial()
        self.assertEqual(out.count("Pause"), 5)


if __name__ == "__main__":
    unittest.main()
