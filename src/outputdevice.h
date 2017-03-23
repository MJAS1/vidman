#ifndef OUTPUTDEVICE_H
#define OUTPUTDEVICE_H


/*! Output device is used to control a parallel or a serial port for trigger
 * signals. */

class OutputDevice
{
public:
    OutputDevice();
    ~OutputDevice();

    enum PortType {
        PORT_NULL,
        PORT_SERIAL,
        PORT_PARALLEL
    };

    void writeData(int trigSignal);
    bool open(PortType port);
    void close();
    bool isEmpty() const;
    void setFd(int);

private:
    OutputDevice(const OutputDevice&);
    OutputDevice& operator=(const OutputDevice&);

    int fd_;
    PortType portType_;
};

#endif // OUTPUTDEVICE_H
