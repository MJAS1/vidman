from sys import argv
import random

if len(argv) != 2:
    print "Usage: " + argv[0] + " <filename>."
    exit()

script, filename = argv

file = open(filename, 'w')
file.truncate();

imgFilename = "../img/button.png"
fadeDuration = 2000
fadeoutTrigCode = 1
fadeinTrigCode = 2
imgTrigCode = 3
standardTrigCode = 4
deviantTrigCode = 5
blackDuration = 1000
minDelay = 1000
maxDelay = 2000
imgDuration = 1000
imgDelay = 1000
x = 400
y = 100

file.write("Object: type=image, id=0, filename="+imgFilename+"\n\n")
trials = []

for i in range(0, 159):
    trial = "Event: type=fadeout, start=0, duration={}, delay={}," \
            " trigcode={}\n".format(fadeDuration, fadeDuration, fadeoutTrigCode) 
    trial += "Delete: start=0, type=freeze\n"
    trial += "Event: type=fadein, start={}, duration={}, delay={}," \
             "trigcode={}\n".format(blackDuration, fadeDuration, 
                                    fadeDuration, fadeinTrigCode)
    trial += "Event: type=detectmotion, trigCode={}\n".format(standardTrigCode)

    delay = random.randint(minDelay, maxDelay)
    trial += "Event: type=image, start={}, x={}, y={}, objectid=0," \
             "delay={}, trigcode={}\n".format(delay, x, y, imgDuration, imgTrigCode)
    trial += "Delete: start={}, type=image, delay={}\n".format(imgDuration, imgDelay)
    trials.append(trial) 

for i in range(0, 39):
    trial = "Event: type=fadeout, start=0, duration={}, delay={}," \
            " trigcode={}\n".format(fadeDuration, fadeDuration, fadeoutTrigCode) 
    trial += "Delete: start=0, type=freeze\n"
    trial += "Event: type=freeze, start=0\n"
    trial += "Event: type=fadein, start={}, duration={}, delay={}," \
             "trigcode={}\n".format(blackDuration, fadeDuration, 
                                    fadeDuration, fadeinTrigCode)
    trial += "Event: type=detectmotion, trigCode={}\n".format(deviantTrigCode)

    delay = random.randint(minDelay, maxDelay)
    trial += "Event: type=image, start={}, x={}, y={}, objectid=0," \
             "delay={}, trigcode={}\n".format(delay, x, y, imgDuration, imgTrigCode)
    trial += "Delete: start={}, type=image, delay={}\n".format(imgDuration, imgDelay)
    trials.append(trial) 

random.shuffle(trials)

for trial in trials:
    file.write(trial)
    file.write("\n")

file.close();
