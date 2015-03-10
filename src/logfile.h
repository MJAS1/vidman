#ifndef LOGFILE_H
#define LOGFILE_H

#include <iostream>
#include <QFile>
#include <QMutex>
#include "timerwithpause.h"

class LogFile
{
public:

    LogFile(const TimerWithPause& timer);
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

    bool active_;

    QFile file_;
    mutable QMutex mutex_;
    const TimerWithPause& timer_;
};


#endif // LOGFILE_H
