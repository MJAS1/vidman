/*
 * logfile.cpp
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

#include <QDateTime>
#include <QTextStream>
#include <QMutexLocker>

#include "logfile.h"

LogFile::LogFile() : active_(false)
{
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

QString LogFile::errorString() const
{
    return file_.errorString();
}

QString LogFile::fileName() const
{
    return file_.fileName();
}
