#ifndef OUTPUTDEVICE_H
#define OUTPUTDEVICE_H

#include <sys/ioctl.h>
#include <sys/io.h>
#include <fcntl.h>
#include <termios.h>
#include <cerrno>
#include <stdio.h>
#include "settings.h"

enum PortType {
    PORT_NULL,
    PORT_SERIAL,
    PORT_PARALLEL
};


//Output device is used to control parallel or serial port for trigger signals.

class OutputDevice
{
public:
    OutputDevice();
    ~OutputDevice();

    void writeData(int trigSignal);
    bool open(PortType port);
    void close();
    bool isEmpty() const;
    void setFd(int);

private:
    int fd;

    Settings settings;
    PortType portType;

};

#endif // OUTPUTDEVICE_H
