#!/usr/bin/env python2
# -*- coding: utf-8 -*-
"""
    test_trialgen.py

    Author: Manu Sutela
    Copyright (C) 2018 BioMag Laboratory, Helsinki University Central Hospital

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
        standards = out.count("trig_code={}".format(tg.TRIG_STANDARD))
        deviants = out.count("trig_code={}".format(tg.TRIG_DEVIANT))
        blanks = out.count("trig_code={}".format(tg.TRIG_BLANK))

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
                if "trig_code={}".format(tg.TRIG_DEVIANT) in line:
                    if first is False:
                        distances.append(dist)
                        dist = 0
                    else:
                        first = False
                elif "trig_code={}".format(tg.TRIG_STANDARD) in line:
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
