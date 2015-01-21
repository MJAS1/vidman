#include <iostream>
#include <QDateTime>
#include <QTextStream>
#include "logfile.h"

LogFile::LogFile(TimerWithPause& timer) : active(false), timer(timer)
{
}

LogFile::~LogFile()
{
    if(file.isOpen())
        file.close();
}

void LogFile::setActive(bool on)
{
    active = on;
}
bool LogFile::isActive() const
{
    return active;
}

bool LogFile::open()
{
       file.setFileName(QDateTime::currentDateTime().toString(QString("yyyy-MM-dd--hh:mm:ss.log")));
       return file.open(QFile::WriteOnly | QFile::Truncate);
}

void LogFile::write(const QString& log)
{
    if(active)
    {
        qint64 elapsedTime = timer.nsecsElapsed();
        QTextStream logStream(&file);
        logStream << "[" << elapsedTime/1000000000 << "s "
                  << (elapsedTime%1000000000)/1000000 << "ms]"
                  << log << "\n";
    }
}

void LogFile::close()
{
    file.close();
}

void LogFile::operator <<(const QString& log)
{
    write(log);
}
