/*
 * logfile.h
 *
 * Author: Manu Sutela
 * Copyright (C) 2018 Aalto University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
