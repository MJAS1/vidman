from sys import argv
import random

random.seed(312)

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
blackScreenTrigCode = 4
missTrigCode = 5
minDelay = 1000
maxDelay = 2000
imgDuration = 700
x = 290
y = 270
target = 250
tolerance = 51

file.write("Object: type=image, id=0, filename="+imgFilename+"\n\n")

trials = []
trials = ["standard"]*400
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

trials[len(trials)/2:len(trials)/2] = ["blackscreen"]

for trial in trials:
    if trial == "standard":
        delay = random.randint(minDelay, maxDelay)
        file.write("Event: type=detectmotion, target={}, tolerance={}, "
                   "trigCode={}, trigcode2={}, start={}\n".format(
                                                            target,
                                                            tolerance,
                                                            standardTrigCode,
                                                            missTrigCode,
                                                            delay))
        file.write("Event: type=image, x={}, y={}, objectid=0," \
                 "delay={}, trigcode={}\n".format(x,
                                                  y,
                                                  imgDuration,
                                                  imgTrigCode))
        file.write("Delete: start={}, type=image\n".format(imgDuration))
        file.write("Delete: start=0, type=detectmotion\n")

    elif trial == "deviant":
        delay = random.randint(minDelay, maxDelay)
        file.write("Event: type=detectmotion, target={}, tolerance={}, "
                   "trigCode={}, trigcode2={}, start={}\n".format(
                                                            target,
                                                            tolerance,
                                                            deviantTrigCode,
                                                            missTrigCode,
                                                            delay))
        file.write("Event: type=freeze, start=0\n")
        file.write("Event: type=image, x={}, y={}, objectid=0, " \
                 "delay={}, trigcode={}\n".format(
                                            x,
                                            y,
                                            imgDuration,
                                            imgTrigCode))
        file.write("Delete: start={}, type=image\n".format(imgDuration))
        file.write("Delete: start=0, type=freeze\n")
        file.write("Delete: start=0, type=detectmotion\n")

    elif trial == "blackscreen":
    	file.write("Pause\n")
        file.write("Event: type=fadeout, start=0, duration=1000, "
                    "delay=1000\n")
        for i in range(250):
            file.write("Event: type=detectmotion, target={}, " 
                        "tolerance={}, trigCode={}, trigcode2={}, "
                        "start={}\n".format(
                                        target,
                                        tolerance,
                                        blackScreenTrigCode,
                                        missTrigCode,
                                        delay))
            file.write("Event: type=image, x={}, y={}, objectid=0, " \
                 "delay={}, trigcode={}\n".format(
                                            x,
                                            y,
                                            imgDuration,
                                            imgTrigCode))
            file.write("Delete: start={}, type=image\n".format(imgDuration))
            file.write("Delete: start=0, type=detectmotion\n")
        file.write("Event: type=fadein, start=0, duration=1000, "
                    "delay=1000\n")
    	file.write("Pause\n")
            
   
file.close();
