## Intro

VidMan has been tested on Ubuntu 14.04 LTS. Some problems with vsync were
encountered on Ubuntu 12.04 LTS but newer versions should be fine. Vsync also
requires a graphics card with OpenGL support. Some older Nvidia drivers caused
the software to sometimes hang while using vsync but this problem should be
fixed on newer drivers. Nvidia drivers for Linux x64 version 331.113 were used
for testing. Vsync can be turned off by modifying the config file usually
located at ~/.config/BECS/VidMan.conf and changing the variable vsync to false.

## Installation

VidMan requires Qt and OpenCV to compile. This branch should work with all Qt
versions starting from Qt 4.8, including Qt 5. It hasn't been properly tested,
so if you face any strange problems, you should switch to the master branch
version and use it with Qt 4.8. OpenCV 2.4.9 was used for testing but newer
versions, including OpenCV 3 should work too.  Other packages required are
libjpeg-dev and libdc1394-22-dev, which can be installed via:

	sudo apt-get install libjpeg-dev libdc1394-22-dev 

if not already installed by default.  

To compile you can either open the project in Qt creator and proceed from there
or from the command line inside the VidMan directory type:

	qmake CONFIG+=release
	make

for release version, and:

	qmake CONFIG+=debug
	make

for debug version. When performing measurements make sure that you are using the
release build, since the debug build might introduce extra lag.

## Usage

VidMan works by applying specific events to each frame grabbed by the video
camera. The starting time and other parameters of each event must be speficied
in the text editor. To declare an event the following syntax is used:

	event: type=flip, start=100, delay=1000, id=1

This declares an event that flips the frame starting 100ms after the previous
event. By setting delay to 1000, the next event starts 1000ms after this event
even if its start is 0. To remove an event you can use the following:

    delete: start=2000, id=1

or

	delete: start=2000, type=flip

The former removes the event with id 1 and the latter removes all events of type
flip. Because there can only exist one flip event at a time, the latter is
simpler in this case. The available events with all possible parameters for each
are listed below:

* **flip**: start, delay, id, trigcode
* **fadein**: start, duration, delay, id, trigcode
* **fadeout**: start, duration, delay, id, trigcode
* **image**: start, x, y, objectid, delay, id, trigcode
* **text**: start, string, color, x, y, delay, id, trigcode
* **rotate**: start, angle, delay, id, trigcode
* **freeze**: start, delay, id, trigcode
* **zoom**: start, scale, duration, delay, id, trigcode
* **record**: start, objectid, delay, duration, id, trigcode
* **playback**: start, objectid, delay, duration, id, trigcode
* **detect motion**: start, delay, id, trigcode

Duration in milliseconds is used for certain events that modify multiple
subsequent frames. Note the difference between duration and delay. If you want
the next event to take place only after the previous has finished, both should
be set to the same value, e.g.:

	event: type=fadeout, start=0, duration=5000, delay=5000
    event: type=freeze

Note also that most parameters are optional. Omitting start for example sets it
to default value 0. 

Each event has a trigcode parameter which is used to specify the code emitted
through serial or parallel port when the event is applied. You need to run the
software with root priviliges in order to gain access to the ports. You also
need to specify which port you want to use by choosing menu->edit->use serial
port or use parallel port.

To use image, record and playback events an object needs to declared. For image
use the following syntax:

	object: type=image, id=0, filename=../imag/button.png

and for record and playback:

	object: type=video, id=0, duration=2000 

Duration is the maximum duration in milliseconds of the video clip recorded by a
record event. This clip can then be played with a playback event, e.g.:

	object: type=video, id=0, duration=2000 
	event: type=record, start=0, duration=2000, delay=2000, objectId=0 
	event: type=playback, start=0, duration=2000, objectId=0
     
The detect motion event works by comparing subsequent frames and counting the
number of changes in pixels. If the number of changes exceeds a threshold value
a triggercode is sent. Depending on the environment, the threshold value might
need to be adjusted. If there is a lot of background noise you should increase
the value from default 10. The value can be set in the config file
~/.config/BECS/VidMan.conf by changing the variable movementSensitivity. To
check how well the motion detection is working, open the motion dialog from
menu->view->motion dialog.