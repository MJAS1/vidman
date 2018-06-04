#ifndef LOGFILE_H
#define LOGFILE_H

#include <QFile>
#include <QMutex>

/*!
 * This class is used to write logfiles during the experiment.
 */
class LogFile : public QObject
{
    Q_OBJECT
public:
    LogFile();

    void setActive(bool on);
    bool isActive() const;
    bool open();
    void close();

    QString errorString() const;
    QString fileName() const;

    void operator<<(const QString& log);

public slots:
    void write(const QString& log);

private:
    LogFile(const LogFile&);
    LogFile& operator=(const LogFile&);

    bool active_;

    QFile file_;
    mutable QMutex mutex_;
};


#endif // LOGFILE_H
