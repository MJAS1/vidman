#ifndef LOGFILE_H
#define LOGFILE_H

#include <iostream>
#include <QFile>
#include "timerwithpause.h"

class LogFile
{
public:
    LogFile(TimerWithPause& timer);
    ~LogFile();
    void setActive(bool on);
    bool isActive() const;
    bool open();
    void close();
    void write(const QString& log);

    void operator<<(const QString& log);
private:
    LogFile(const LogFile&);
    LogFile& operator=(const LogFile&);

    bool active;
    QFile file;
    TimerWithPause& timer;
};


#endif // LOGFILE_H
