/*
 * outputdevice.h
 *
 * Author: Manu Sutela
 * Copyright (C) 2018 Aalto University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
