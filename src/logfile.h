#ifndef LOGFILE_H
#define LOGFILE_H

#include <QFile>
#include <QMutex>

class LogFile
{
public:

    LogFile();
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
};


#endif // LOGFILE_H
