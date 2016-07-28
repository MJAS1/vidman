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

trials = ["normal"]*5

for i in range(0, 79):
    ls = ["normal"]*4
    ls.append("deviant")
    random.shuffle(ls)
    trials.extend(ls)

for i in range(len(trials)-1):
    if trials[i] == "deviant" and trials[i+1] == "deviant":
        trials[i], trials[i-1] = trials[i-1], trials[i]

for trial in trials:
    if trial == "normal":
        file.write("Event: type=fadeout, start=0, duration={}, delay={}," \
                " trigcode={}\n".format(fadeDuration, fadeDuration, fadeoutTrigCode))
        file.write("Delete: start=0, type=freeze\n")
        file.write("Event: type=fadein, start={}, duration={}, delay={}," \
                 "trigcode={}\n".format(blackDuration, fadeDuration, 
                                        fadeDuration, fadeinTrigCode))
        file.write("Event: type=detectmotion, trigCode={}\n".format(standardTrigCode))

        delay = random.randint(minDelay, maxDelay)
        file.write("Event: type=image, start={}, x={}, y={}, objectid=0," \
                 "delay={}, trigcode={}\n".format(delay, x, y, imgDuration, imgTrigCode))
        file.write("Delete: start={}, type=image, delay={}\n".format(imgDuration, imgDelay))
        file.write("\n")

    else:
        file.write("Event: type=fadeout, start=0, duration={}, delay={}," \
                " trigcode={}\n".format(fadeDuration, fadeDuration, fadeoutTrigCode))
        file.write("Delete: start=0, type=freeze\n")
        file.write("Event: type=freeze, start=0\n")
        file.write("Event: type=fadein, start={}, duration={}, delay={}," \
                 "trigcode={}\n".format(blackDuration, fadeDuration, 
                                        fadeDuration, fadeinTrigCode))
        file.write("Event: type=detectmotion, trigCode={}\n".format(deviantTrigCode))

        delay = random.randint(minDelay, maxDelay)
        file.write("Event: type=image, start={}, x={}, y={}, objectid=0," \
                 "delay={}, trigcode={}\n".format(delay, x, y, imgDuration, imgTrigCode))
        file.write("Delete: start={}, type=image, delay={}\n".format(imgDuration, imgDelay))
        file.write("\n")

file.close();
