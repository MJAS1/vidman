#include <QMessageBox>
#include <cerrno>
#include <iostream>
#include "outputdevice.h"

OutputDevice::OutputDevice() : portType_(PORT_NULL)
{
}

void OutputDevice::writeData(int trigCode)
{
    switch (portType_) {
        case PORT_PARALLEL:
            outb(trigCode, settings_.printerPortAddr);
            break;

        case PORT_SERIAL:
            if(ioctl(fd_, TIOCMSET, &trigCode) == -1) {
                fprintf(stderr, "Cannot open port: %s\n", strerror(errno));
                portType_ = PORT_NULL;
            }
            break;

        default:
            break;
    }
}

bool OutputDevice::open(PortType port)
{
    switch (port) {
        case PORT_PARALLEL:
            if(ioperm(settings_.printerPortAddr, 1, 1)) {
            //if((fd_ = ::open("/dev/lp0", O_RDWR)) < 1) {
                portType_ = PORT_NULL;
                std::cerr << "Cannot get the port. May be you should run this program as root" << std::endl;
            }
            else
                portType_ = PORT_PARALLEL;
            break;

        case PORT_SERIAL:
            if((fd_ = ::open("/dev/ttyUSB0", O_RDWR)) < 1) {
                portType_ = PORT_NULL;
                fd_ = -1;
                std::cerr << "Cannot open USB port. May be you should run this program as root" << std::endl;
            }
            else
                portType_ = PORT_SERIAL;
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

void OutputDevice::setFd(int fileDescription)
{
    fd_ = fileDescription;
}

void OutputDevice::close()
{
    portType_ = PORT_NULL;
}
