from sys import argv
import random

if len(argv) != 2:
    print "Usage: " + argv[0] + " <filename>."
    exit()

script, filename = argv

file = open(filename, 'w')
file.truncate();

imgFilename = "../img/button.png"
imgTrigCode = 1
standardTrigCode = 2
deviantTrigCode = 3
minDelay = 1000
maxDelay = 2000
imgDuration = 700
x = 400
y = 200
target = 250
tolerance = 51

file.write("Object: type=image, id=0, filename="+imgFilename+"\n\n")

trials = []
trials = ["standard"]*300
trials.extend(["deviant"]*100)
random.shuffle(trials)
#Start each trial with only standards
trials[0:0] = 20*["standard"]

#Make sure a fixed number of standard runs follow each deviant
i = 0
while i < len(trials):
    if trials[i] == "deviant": 
        trials[i+1:i+1] = 5*["standard"]
        i = i + 6
    else:
        i = i + 1

for trial in trials:
    if trial == "standard":
        delay = random.randint(minDelay, maxDelay)
        file.write("Event: type=detectmotion, target={}, tolerance={}, trigCode={}, start={}\n".format(target, tolerance, standardTrigCode, delay))
        file.write("Event: type=image, x={}, y={}, objectid=0," \
                 "delay={}, trigcode={}\n".format(x, y, imgDuration, imgTrigCode))
        file.write("Delete: start={}, type=image\n".format(imgDuration))
        file.write("Delete: start=0, type=detectmotion\n")

    else:
        delay = random.randint(minDelay, maxDelay)
        file.write("Event: type=detectmotion, target={}, tolerance={}, trigCode={}, start={}\n".format(target, tolerance, deviantTrigCode, delay))
        file.write("Event: type=freeze, start=0\n")
        file.write("Event: type=image, x={}, y={}, objectid=0," \
                 "delay={}, trigcode={}\n".format(x, y, imgDuration, imgTrigCode))
        file.write("Delete: start={}, type=image\n".format(imgDuration))
        file.write("Delete: start=0, type=freeze\n")
        file.write("Delete: start=0, type=detectmotion\n")

file.close();
