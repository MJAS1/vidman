#include <QDateTime>
#include <QTextStream>
#include "logfile.h"

LogFile::LogFile(const TimerWithPause& timer) : active(false), timer(timer)
{
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
       file.open(QFile::WriteOnly | QFile::Truncate);
}

void LogFile::write(const QString& log)
{
    if(active)
    {
        qint64 elapsedTime = timer.elapsed();
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
