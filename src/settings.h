/*
 * settings.h
 *
 * Original file:
 * Author: Andrey Zhdanov
 * Copyright (C) 2014 BioMag Laboratory, Helsinki University Central Hospital
 *
 * Modifications:
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

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QSettings>
#include <QMutex>

//! Application-wide settings preserved across multiple invocations.
/*!
 * This class contains application-wide settings read from disc. To read the
 * settings simply create the instance of this class and read the values from
 * the corresponding public variables of the class. The settings are supposed
 * to be read only; they can be changed by manually editing the text file
 * between program invocations but they should stay constant for the whole
 * lifetime of a single program instance. The class should be completely
 * thread-safe (CURRENTLY IT IS NOT).
 */

class Settings {
    // TODO: make member variables immutable as much as possible
    // TODO: make the class thread-safe
    // TODO: implement singleton pattern?
public:
    Settings();

    void            setValue(const QString &key, const QVariant &value);

    // video
    int				jpgQuality;
    int             fps;
    int             movementThreshold;
    int             videoWidth;
    int             flipCode;
    int             defaultTrig1;
    int             defaultTrig2;
    bool			color;
    bool            vsync;
    bool            flip;
    bool            fixPoint;
    uint32_t        whitebalance;
    uint32_t        shutter;
    uint32_t        gain;

    // misc
    char			storagePath[500];
    int             printerPortAddr;

private:

    QSettings       settings_;
};

#endif /* SETTINGS_H_ */
