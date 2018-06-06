/*
 * eventparser.h
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

#ifndef EVENTPARSER_H
#define EVENTPARSER_H

#include <memory>
#include <fstream>
#include <QObject>
#include <QMap>
#include <opencv2/opencv.hpp>

#include "event.h"
#include "settings.h"

using std::shared_ptr;
using std::ifstream;
class EventContainer;


struct Frame{
    cv::Mat data_;
    uint8_t trigCode_;
};

/*!
 * VideoObject is used to create record- and playback-events. It contains the
 * frames belonging to the particular video and the duration in milliseconds.
 */
struct VideoObject {
    QList<Frame> frames_;
    int duration_;
};

struct EventAttributes {
    int start_ = 0;
    int duration_ = 0;
    int delay_ = 0;
    int x_ = 0;
    int y_ = 0;
    int objectId_ = 0;
    int eventId_ = -1;
    int angle_ = 0;
    int trigCode_ = 0;
    int successCode_ = 0;
    int failCode_ = 0;
    int target_ = 0;
    int tolerance_ = 0;
    int axis_ = 1;
    float scale_ = 1;
    bool objectIdOk_ = false;
    QString string_;
    QString fileName_;
    QString objectType_;
    cv::Scalar color_ = cv::Scalar(0, 0, 0);
    Event::EventType type_ = Event::EVENT_NULL;
};

/*!
 * This class parses events from a QStringList and stores them in an
 * EventContainer. The QStringList is created from MainWindow's TextEdit when
 * start is clicked. LoadEvents returns false and emits an error message if the
 * input is incorrectly formatted.
 */
class EventParser : public QObject
{
    Q_OBJECT
public:
    bool loadEvents(const QStringList &strList, EventContainer &events,
                    MainWindow* window = nullptr);

signals:
    // Emits an error message if the input string is incorrect.
    void error(const QString&) const;

private:
    bool parseEvent(const QString &str, EventContainer& events) const;
    bool parseObject(const QString &str);
    bool parseDelEvent(const QString &str, EventContainer& events) const;
    bool parseEventParam(const QString &param, const QString &value,
                         EventAttributes &evAttr) const;
    bool parseObjectParam(const QString &param, const QString &value,
                          EventAttributes &evAttr) const;
    bool assertHeader(ifstream &data) const;
    bool loadVideo(const QString &filename,
                   const shared_ptr<VideoObject> &video) const;


    EventPtr createEvent(const EventAttributes &evAttr) const;
    EventPtr createDelEvent(EventAttributes &evAttr) const;

    float toFloat(const QString &str, const QString &param) const;
    int toInt(const QString &str, const QString &param) const;

    Settings settings_;
    int currentLine_;

    //VideoObjects shared between a RecordEvent and a PlaybackEvent
    QMap<int, shared_ptr<VideoObject>>  videoObjects_;
    QMap<int, cv::Mat>                  imageObjects_;
};

#endif // EVENTPARSER_H
