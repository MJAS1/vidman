#include <sys/ioctl.h>
#include <sys/io.h>
#include <fcntl.h>
#include <termios.h>
#include <cerrno>
#include <iostream>
#include <string.h>
#include "outputdevice.h"
#include "settings.h"

using namespace std;

OutputDevice::OutputDevice() : portType_(PORT_NULL)
{
    Settings settings;
    open(settings.portType);
}

OutputDevice::~OutputDevice()
{
}

void OutputDevice::writeData(int trigCode)
{
    Settings settings;

    switch (portType_) {
        case PORT_PARALLEL:
            outb(trigCode, settings.printerPortAddr);
            break;

        case PORT_SERIAL:
            if(ioctl(fd_, TIOCMSET, &trigCode) == -1) {
                cerr << "Cannot open port: " <<  strerror(errno) << endl;
                portType_ = PORT_NULL;
                settings.setValue("misc/port_type", PORT_NULL);
            }
            break;

        default:
            break;
    }
}

bool OutputDevice::open(PortType port)
{
    Settings settings;

    switch (port) {
        case PORT_PARALLEL:
            if(ioperm(settings.printerPortAddr, 1, 1)) {
                portType_ = PORT_NULL;
                settings.setValue("misc/port_type", PORT_NULL);
                cerr << "Cannot get the port. Maybe you should run this"
                        "program as root." << endl;
            }
            else {
                portType_ = PORT_PARALLEL;
                settings.setValue("misc/port_type", PORT_PARALLEL);
            }
            break;

        case PORT_SERIAL:
            if((fd_ = ::open("/dev/ttyUSB0", O_RDWR)) < 1) {
                portType_ = PORT_NULL;
                settings.setValue("misc/port_type", PORT_NULL);
                fd_ = -1;
                cerr << "Cannot open USB port. Maybe you should run this"
                        "program as root." << endl;
            }
            else {
                portType_ = PORT_SERIAL;
                settings.setValue("misc/port_type", PORT_SERIAL);
            }
            break;

        default:
            break;
    }

    return true;
}

bool OutputDevice::isEmpty() const
{
    if(!portType_)
        return true;

    return false;
}

void OutputDevice::setFd(int fd)
{
    fd_ = fd;
}

void OutputDevice::close()
{
    portType_ = PORT_NULL;
}
