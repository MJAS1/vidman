#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from sys import argv, exit
import numpy as np
import random

random.seed(312)

#%% Constants
IMG = "../img/button.png"
TRIG_IMG = 1
TRIG_STANDARD = 2
TRIG_DEVIANT = 3
TRIG_BLANK = 4
TRIG_STOP = 5
IMG_DURATION = 1200
X = 330
Y = 150
TARGET = 575
TOLERANCE = 76
MIN_DELAY = 1000
MAX_DELAY = 2000


#%%
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
    numpy.array
    """
    if n_deviants and (min_distance > (n_standards + n_deviants) / n_deviants):
        raise ValueError("Minimum distance too high.")
        
    initial_standards = n_standards - min_distance * n_deviants
    block = np.array(["standard"]*initial_standards + ["deviant"]*n_deviants)
    block = np.random.permutation(block)
    deviants = np.where(block == "deviant")[0].repeat(min_distance)
    
    #Insert min_distance number of standards after each deviant
    block = np.insert(block, deviants+1, "standard")
    
    return block

    
#%%
def standard_condition(trig):
    delay = random.randint(MIN_DELAY, MAX_DELAY)
    txt = "Event: type=detectmotion, target={}, tolerance={}, trigcode={}," \
          "trigcode2={}, start={}\n".format(TARGET, TOLERANCE, trig,
                                            TRIG_STOP, delay)
    txt += "Event: type=image, x={}, y={}, objectid=0, delay={}," \
           "trigcode={}\n".format(X, Y, IMG_DURATION, TRIG_IMG)
    txt += "Delete: start=0, type=image\n"
    txt += "Delete: start=0, type=detectmotion\n\n"
    
    return txt


#%%
def deviant_condition(trig):
    delay = random.randint(MIN_DELAY, MAX_DELAY)     
    txt = "Event: type=detectmotion, target={}, tolerance={}, trigcode={}, " \
          "trigcode2={}, start={}\n".format(TARGET, TOLERANCE, trig,
                                            TRIG_STOP, delay)
    txt += "Event: type=freeze, start=0\n "
    txt += "Event: type=image, x={}, y={}, objectid=0, delay={}, " \
           "trigcode={}\n".format(X, Y, IMG_DURATION, TRIG_IMG)
    txt += "Delete: start=0, type=image\n"
    txt += "Delete: start=0, type=freeze\n"
    txt += "Delete: start=0, type=detectmotion\n\n"
    
    return txt


#%%
def create_trial():
    """
    Returns
    -------
    string
        Script that can be run by the vidman application.
    """
    
    """
    First block: start with 20 standard conditions then 250 conditions with 10% 
    deviant conditions.
    Second block: 250 conditions with 10% deviants.
    Third block: Black screen with 250 standard conditions.
    Fourth block: 250 conditions with 10% deviants.
    Fifth block: 250 conditions with 10% deviants.
    """
    b1 = np.concatenate([create_block(20, 0, 0), create_block(225, 25, 5)])
    b2 = create_block(225, 25, 5)
    b3 = ["fade out"] + ["blank"]*250 + ["fade in"]
    b4 = create_block(225, 25, 5)
    b5 = create_block(225, 25, 5)
    
    blocks = [b1, b2, b3, b4, b5]                              
    out = ""
    for block in blocks:
        for condition in block:
            if condition == "standard":
                out += standard_condition(TRIG_STANDARD)
            elif condition == "deviant":
                out += deviant_condition(TRIG_DEVIANT)
            elif condition == "blank":
                out += standard_condition(TRIG_BLANK)
            elif condition == "fade out":
                out += "Event: type=fadeout, start=0, duration=1000, delay=1000\n\n"
            elif condition == "fade in":
                out += "Event: type=fadein, start=0, duration=1000, delay=1000\n\n"
    
        out += "Pause\n\n"
        
    return out
    

#%%    
if __name__ == '__main__':
    if len(argv) != 2:
        print "Usage: " + argv[0] + " <filename>."
        exit()

    script, filename = argv
    
    out = create_trial()
    
    file = open(filename, 'w')
    file.truncate();
    file.write("Object: type=image, id=0, filename="+IMG+"\n\n")
    file.write(out)            
    file.close();
