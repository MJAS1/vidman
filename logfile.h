#ifndef LOGFILE_H
#define LOGFILE_H

#include <iostream>
#include <QFile>
#include "timerwithpause.h"

class LogFile
{
public:
    LogFile(const TimerWithPause& timer);
    void setActive(bool on);
    bool isActive() const;
    bool open();
    void close();
    void write(const QString& log);

    void operator<<(const QString& log);
private:
    bool active;
    QFile file;
    const TimerWithPause& timer;
};


#endif // LOGFILE_H
