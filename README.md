
VidMan has been tested on Ubuntu 14.04 LTS. Some problems with vsync were
encountered on Ubuntu 12.04 LTS but newer versions should be fine. Vsync also
requires a graphics card with OpenGL support. Some older Nvidia drivers caused
the software to sometimes hang while using vsync but this problem should be
fixed on newer drivers. Nvidia drivers for Linux x64 version 331.113 were used
for testing. Vsync can be turned off by modifying the config file usually
located at ~/.config/BECS/VidMan.conf and changing the variable vsync to false.

## Installation

VidMan requires Qt and OpenCV to compile. Unfortunately, due to some changes in
the implementation of OpenGL in Qt, Qt 5 doesn't seem to be compatible with
VidMan at the moment. All the testing was performed using Qt 4.8, which should
be available from Qt archives (<http://download.qt.io/archive/>). OpenCV 2.4.9
was used for testing but newer versions, including OpenCV 3 should work too.
Other packages required are libjpeg-dev and libdc1394-22-dev, which can be
installed via:

	sudo apt-get install libjpeg-dev libdc1394-22-dev 

if not already installed by default.  

To compile you can either open the project in Qt creator and proceed from there
or from the command line inside the VidMan directory type:

	qmake CONFIG+=release
	make

For release version, and:

	qmake CONFIG+=debug
	make

for debug version.
