#include <iostream>
#include <QDateTime>
#include <QTextStream>
#include <QMutexLocker>
#include "logfile.h"

LogFile::LogFile() : active_(false)
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
    file_.setFileName(QString("../logs/") + QDateTime::currentDateTime().toString(QString("yyyy-MM-dd--hh:mm:ss.log")));
    //file_ = QFile()
    return file_.open(QFile::WriteOnly | QFile::Truncate);
}

void LogFile::write(const QString& log)
{
    mutex_.lock();
    if(active_) {
        QTextStream logStream(&file_);
        logStream << log << "\n";
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
