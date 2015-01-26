#include <iostream>
#include <QDateTime>
#include <QTextStream>
#include <QMutexLocker>
#include "logfile.h"

LogFile::LogFile(const TimerWithPause& timer) : active_(false), timer_(timer)
{
}

LogFile::~LogFile()
{
    if(file_.isOpen())
        file_.close();
}

void LogFile::setActive(bool on)
{
    mutex_.lock();
    active_ = on;
    mutex_.unlock();
}
bool LogFile::isActive() const
{
    QMutexLocker locker(&mutex_);
    return active_;
}

bool LogFile::open()
{
    QMutexLocker locker(&mutex_);
    file_.setFileName(QDateTime::currentDateTime().toString(QString("yyyy-MM-dd--hh:mm:ss.log")));
    return file_.open(QFile::WriteOnly | QFile::Truncate);
}

void LogFile::write(const QString& log)
{
    mutex_.lock();
    if(active_)
    {
        qint64 elapsedTime = timer_.nsecsElapsed();
        QTextStream logStream(&file_);
        logStream << "[" << elapsedTime/1000000000 << "s "
                  << (elapsedTime%1000000000)/1000000 << "ms]"
                  << log << "\n";
    }
    mutex_.unlock();
}

void LogFile::close()
{
    mutex_.lock();
    file_.close();
    mutex_.unlock();
}

void LogFile::operator <<(const QString& log)
{
    write(log);
}
