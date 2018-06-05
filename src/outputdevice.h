#ifndef OUTPUTDEVICE_H
#define OUTPUTDEVICE_H

#include "settings.h"

/*!
 * This class is used to control a parallel or a serial port for trigger
 * signals. To make writeData() work, open() should be called by the same
 * thread that is used to write to the output port. May require root priviliges
 * to work correctly.
 */
class OutputDevice : public QObject
{
    Q_OBJECT
public:
    OutputDevice();

    enum PortType {
        PORT_NULL,
        PORT_SERIAL,
        PORT_PARALLEL
    };

    void close();
    bool isEmpty() const;

public slots:
    bool open(OutputDevice::PortType port);

    void writeData(int);

private:
    int fd_;

    Settings settings_;
    PortType portType_;

};

#endif // OUTPUTDEVICE_H
