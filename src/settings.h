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

// Most code in this class from https://github.com/andreyzhd/VideoMEG
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
