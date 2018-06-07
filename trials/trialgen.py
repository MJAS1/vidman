#!/usr/bin/env python2
# -*- coding: utf-8 -*-

"""
    trialgen.py

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
from sys import argv, exit
import random

import numpy as np

random.seed(312)
np.random.seed(312)

# %% Constants
IMG = "../img/button.png"
TRIG_IMG = 1
TRIG_STANDARD = 2
TRIG_DEVIANT = 3
TRIG_BLANK = 4
TRIG_SUCCESS = 5
TRIG_FAIL = 6
IMG_DURATION = 1200
IMG_X = 390
IMG_Y = 215
DETECTOR_X = 375
DETECTOR_Y = 240
TARGET = 575
TOLERANCE = 76
MIN_DELAY = 1000
MAX_DELAY = 2000


# %%
def create_block(n_standards, n_deviants, min_distance):
    """
    Parameters
    ----------
    n_standards : int
        Number of standard condition in the block

    n_deviants : int
        Number of deviant conditions in the block

    min_distance : int
        Minimun distance between successive deviant conditions

    Returns
    -------
    block : numpy.array
    """
    if n_deviants and (min_distance > (n_standards + n_deviants) / n_deviants):
        raise ValueError("Minimum distance too high.")

    initial_standards = n_standards - min_distance * n_deviants
    block = np.array(["standard"]*initial_standards + ["deviant"]*n_deviants)
    block = np.random.permutation(block)
    deviants = np.where(block == "deviant")[0].repeat(min_distance)

    # Insert min_distance number of standards after each deviant
    block = np.insert(block, deviants+1, "standard")

    return block


# %%
def standard_condition(trig):
    """
    Parameters
    ----------
    trig : int
        Trigger code for standard trials

    Returns
    -------
    trial : str
    """
    delay = random.randint(MIN_DELAY, MAX_DELAY)
    trial = "Event: type=detect_motion, target={}, tolerance={}, "\
            "trig_code={}, success_code={}, fail_code={}, x={}, y={}, "\
            "start={}\n".format(TARGET, TOLERANCE, trig, TRIG_SUCCESS,
                                TRIG_FAIL, DETECTOR_X, DETECTOR_Y, delay)
    trial += "Event: type=image, x={}, y={}, object_id=0, delay={}, " \
             "trig_code={}\n".format(IMG_X, IMG_Y, IMG_DURATION, TRIG_IMG)
    trial += "Delete: start=0, type=image\n"
    trial += "Delete: start=0, type=detect_motion\n\n"

    return trial


# %%
def deviant_condition(trig):
    """
    Parameters
    ----------
    trig : int
        Trigger code for deviant trials

    Returns
    -------
    trial : str
    """
    delay = random.randint(MIN_DELAY, MAX_DELAY)
    trial = "Event: type=detect_motion, target={}, tolerance={}, "\
            "trig_code={}, success_code={}, fail_code={}, x={}, y={}, "\
            "start={}\n".format(TARGET, TOLERANCE, trig, TRIG_SUCCESS,
                                TRIG_FAIL, DETECTOR_X, DETECTOR_Y, delay)
    trial += "Event: type=freeze, start=0\n "
    trial += "Event: type=image, x={}, y={}, object_id=0, delay={}, " \
             "trig_code={}\n".format(IMG_X, IMG_Y, IMG_DURATION, TRIG_IMG)
    trial += "Delete: start=0, type=image\n"
    trial += "Delete: start=0, type=freeze\n"
    trial += "Delete: start=0, type=detect_motion\n\n"

    return trial


# %%
def create_trial():
    """
    Returns
    -------
    out : str
        Script that can be run by the vidman application.
    """

    # First block: Starts with 20 standard trials after which 250 trials of
    # which 10% are deviants.
    # Second block: 250 trials where 10% are deviants.
    # Third block: 250 blank trials.
    # Fourth block: 250 trials where 10% are deviants.
    # Fifth block: 250 trials where 10% are deviants.
    b1 = np.concatenate([create_block(20, 0, 0), create_block(225, 25, 5)])
    b2 = create_block(225, 25, 5)
    b3 = ["fade_out"] + ["blank"]*250 + ["fade_in"]
    b4 = create_block(225, 25, 5)
    b5 = create_block(225, 25, 5)

    out = ""
    for block in [b1, b2, b3, b4, b5]:
        for condition in block:
            if condition == "standard":
                out += standard_condition(TRIG_STANDARD)
            elif condition == "deviant":
                out += deviant_condition(TRIG_DEVIANT)
            elif condition == "blank":
                out += standard_condition(TRIG_BLANK)
            elif condition == "fade_out":
                out += "Event: type=fade_out, start=0, duration=1000, "\
                       "delay=1000\n\n"
            elif condition == "fade_in":
                out += "Event: type=fade_in, start=0, duration=1000, "\
                       "delay=1000\n\n"

        out += "Pause\n\n"

    return out


# %%
if __name__ == '__main__':
    if len(argv) != 2:
        print "Usage: " + argv[0] + " <filename>."
        exit()

    script, filename = argv

    out = create_trial()

    file = open(filename, 'w')
    file.write("Object: type=image, id=0, filename="+IMG+"\n\n")
    file.write(out)
    file.close()
