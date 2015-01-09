#include <QMessageBox>
#include <cerrno>
#include <iostream>
#include "outputdevice.h"

OutputDevice::OutputDevice()
{
}

OutputDevice::~OutputDevice()
{
}

void OutputDevice::writeData(int trigCode) const
{
    switch (portType)
    {
        case PORT_PARALLEL:
            outb(trigCode, settings.printerPortAddr);
            break;

        case PORT_SERIAL:
            if(ioctl(fd, TIOCMSET, &trigCode) == -1)
            {
                fprintf(stderr, "Cannot open port: %s\n", strerror(errno));
            }
            break;

        default:
            break;
    }
}

bool OutputDevice::open(PortType port)
{
    switch (port)
    {
        case PORT_PARALLEL:
            if(ioperm(settings.printerPortAddr, 1, 1))
            {
                QMessageBox msgBox;
                msgBox.setText("Cannot get the port. May be you should run this program as root\n");
                msgBox.exec();
                portType = PORT_NULL;
                return false;
            }
            portType = PORT_PARALLEL;
            break;

        case PORT_SERIAL:
            if((fd = ::open("/dev/ttyUSB0", O_RDWR)) < 1)
            {
                QMessageBox msgBox;
                msgBox.setText("Cannot open USB port.");
                msgBox.exec();
                portType = PORT_NULL;
                fd = -1;
                return false;
            }
            portType = PORT_SERIAL;
            break;

        default:
            break;
    }

    return true;
}

bool OutputDevice::isEmpty() const
{
    if(!portType)
        return true;

    return false;
}

void OutputDevice::setFd(int fileDescription)
{
    fd = fileDescription;
}

void OutputDevice::close()
{
    portType = PORT_NULL;
}
