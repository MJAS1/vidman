#include <iostream>
#include <QStringList>
#include <sys/ioctl.h>
#include "eventparser.h"
#include "eventcontainer.h"

using std::move;

EventParser::EventParser()
{
}

bool EventParser::loadEvents(const QStringList &strList, EventContainer& events)
{
    imageObjects_.clear();
    videoObjects_.clear();
    events.clear();

    //Start parsing the events, line by line
    for(int i = 0; i < strList.size(); i++)
    {
        //The string should be of the form "Event: type=.., start=.., etc.."
        //Split from ':', remove whitespaces and convert to lower case
        QStringList split = strList[i].split(':');
        QString str = split[0].toLower().replace(" ", "").simplified();

        if(split.size() > 1) {
            if(str == "event") {
                if(!parseEvent(split[1], events, i+1))
                    return false;
            }
            else if(str == "object") {
                if(!parseObject(split[1], i+1))
                    return false;
            }
            else if(str == "delete") {
                if(!parseDelEvent(split[1], events, i+1))
                    return false;
            }
            //Ignore comments
            else if(str[0] == '#' || split[0].simplified() == "");
            else {
                emit error(QString("Error: couldn't understand '%1' in line %2.").arg(split[0]).arg(i+1));
                return false;
            }
        }
        else if(!str.isEmpty()) {
            emit error(QString("Error: couldn't understand line %1.").arg(i+1));
            return false;
        };
    }
    return true;
}

bool EventParser::parseEvent(const QString &str, EventContainer& events, int lineNumber)
{
    //Split the string to get a list of strings of the form param=val e.g. {"type=...", "start=...", etc..}
    QStringList strList = str.split(',');

    //Event parameters
    start_ = duration_ = delay_ = x_ = y_ = objectId_ = angle_ = trigCode_ = 0;
    eventId_ = -1;
    scale_ = 1;
    objectIdOk_ = false;
    color_ = cv::Scalar(0, 0, 0);
    type_ = Event::EVENT_NULL;

    //Fill all the parameters required to create an event
    for(int i = 0; i < strList.size(); i++) {
        if(strList[i].contains("=")) {
            QStringList split = strList[i].split('=');
            if(!parseEventParam(split[0], split[1], lineNumber))
                return false;
        }
    }

    //Create the new event with the acquired parameters
    EventPtr ev;
    if(!createEvent(ev, lineNumber))
        return false;
    events.append(move(ev));

    return true;
}

bool EventParser::parseDelEvent(const QString &str, EventContainer& events, int lineNumber)
{
    QStringList strList = str.split(',');

    //Event parameters
    eventId_ = -1;
    trigCode_ = start_ = delay_ = 0;
    type_ = Event::EVENT_NULL;

    for(int i = 0; i < strList.size(); i++) {
        if(strList[i].contains("=")) {
            QStringList split = strList[i].split("=");
            QString param = split[0];
            QString value = split[1];

            if(!parseEventParam(param, value, lineNumber))
                return false;
        }
    }

    if(eventId_ > -1 && type_) {
        emit error(QString("Error: remove event declared with id and type in line %1.").arg(lineNumber));
        return false;
    }
    else if(eventId_ > -1) {
        EventPtr ev(new DelEvent(start_, delay_, eventId_, trigCode_));
        ev->appendLog(QString("Event ID %1 removed. ").arg(eventId_));
        events.append(move(ev));
    }
    else {
        EventPtr ev(new DelEvent(start_, delay_, type_, trigCode_));

        switch(type_) {
        case Event::EVENT_FLIP:
            ev->appendLog(QString("Flip event removed. "));
            break;
        case Event::EVENT_FADEIN:
            ev->appendLog(QString("Fade in event removed. "));
            break;
        case Event::EVENT_FADEOUT:
            ev->appendLog(QString("Fade out event removed. "));
            break;
        case Event::EVENT_IMAGE:
            ev->appendLog(QString("Image event removed. "));
            break;
        case Event::EVENT_TEXT:
            ev->appendLog(QString("Text event removed. "));
            break;
        case Event::EVENT_ROTATE:
            ev->appendLog(QString("Rotate event removed. "));
            break;
        case Event::EVENT_FREEZE:
            ev->appendLog(QString("Freeze event removed. "));
            break;

        case Event::EVENT_ZOOM:
            ev->appendLog(QString("Zoom event removed."));
            break;
        case Event::EVENT_PLAYBACK:
            ev->appendLog(QString("Playback event removed."));
            break;
        case Event::EVENT_RECORD:
            ev->appendLog(QString("Record event removed."));
            break;

        default:
            emit error(QString("Error: remove event declared without id or type in line %1").arg(lineNumber));
            return false;
        }

        events.append(move(ev));
    }

    return true;
}

bool EventParser::parseObject(const QString &str, int lineNumber)
{
    QStringList strList = str.split(',');

    objectType_.clear();
    fileName_.clear();

    objectId_ = duration_ = 0;

    for(int i = 0; i < strList.size(); i++) {
        if(strList[i].contains("=")) {
            QStringList split = strList[i].split('=');
            QString param = split[0];
            QString value = split[1];

            if(!parseObjectParam(param, value, lineNumber))
                return false;
        }
    }

    if(objectType_ == "image") {
        cv::Mat file(cv::imread(fileName_.toStdString(), CV_LOAD_IMAGE_UNCHANGED));
        if(file.empty()) {
            emit error(QString("Error: couldn't load image file '%1'.").arg(fileName_));
            return false;
        }
        imageObjects_[objectId_] = file;
    }
    else if(objectType_ == "video"){
        shared_ptr<VideoObject> videoObject(new VideoObject);
        videoObject->duration_ = duration_;

        //Reserve enough memory to hold all the frames. This is necessary to
        //make sure that large blocks of memory don't need to be reallocated
        //while recording.
        videoObject->frames_.reserve(duration_/1000*settings_.fps + 10);
        videoObjects_.insert(objectId_, videoObject);
    }
    else if(objectType_ == "") {
        emit error(QString("Error: no type declared for object in line %1").arg(lineNumber));
        return false;
    }
    else {
        emit error(QString("Error: invalid type '%1' for object in line %1").arg(objectType_).arg(lineNumber));
        return false;
    }
    return true;
}

bool EventParser::parseEventParam(const QString &p, const QString &v, int lineNumber)
{
    QString param = p.toLower().replace(" ", "");
    QString value = v.toLower().replace(" ", "");

    if(param == "type") {
        if(value == "flip") type_ = Event::EVENT_FLIP;
        else if(value == "fadein") type_ = Event::EVENT_FADEIN;
        else if(value == "fadeout") type_ = Event::EVENT_FADEOUT;
        else if(value == "image") type_ = Event::EVENT_IMAGE;
        else if(value == "text") type_ = Event::EVENT_TEXT;
        else if(value == "freeze") type_ = Event::EVENT_FREEZE;
        else if(value == "rotate") type_ = Event::EVENT_ROTATE;
        else if(value == "detectmotion") type_ = Event::EVENT_DETECT_MOTION;
        else if(value == "zoom") type_ = Event::EVENT_ZOOM;
        else if(value == "record") type_ = Event::EVENT_RECORD;
        else if(value == "playback") type_ = Event::EVENT_PLAYBACK;
        else {
            emit error(QString("Error: invalid type '%1' in line %2.").arg(v).arg(lineNumber));
            return false;
        }
    }
    else if(param == "start") {
        if((start_ = toInt(v, lineNumber, QString("start time"))) == -1)
            return false;
    }
    else if(param == "duration") {
        if((duration_ = toInt(v, lineNumber, QString("duration"))) == -1)
            return false;
    }
    else if(param == "x") {
        if((x_ = toInt(v, lineNumber, QString("x-coordinate"))) == -1)
            return false;
    }
    else if(param == "y") {
        if((y_ = toInt(v, lineNumber, QString("y-coordinate"))) == -1)
            return false;
    }
    else if(param == "objectid") {
        if((objectId_ = toInt(v, lineNumber, QString("objectID"))) == -1)
            return false;
        objectIdOk_ = true;
    }
    else if(param == "string") {
        string_=v;
    }
    else if(param == "angle") {
        if((angle_ = toInt(v, lineNumber, QString("angle"))) == -1)
            return false;
    }
    else if(param == "id") {
        if((eventId_ = toInt(v, lineNumber, QString("id"))) == -1)
            return false;
    }
    else if(param == "delay") {
        if((delay_ = toInt(v, lineNumber, QString("delay"))) == -1)
            return false;
    }
    else if(param == "trigcode") {
        if(value == "dtr") trigCode_ = TIOCM_DTR;
        else if(value == "rts") trigCode_ = TIOCM_RTS;
        else if((trigCode_ = toInt(v, lineNumber, QString("trigcode"))) == -1)
            return false;
    }
    else if(param == "scale") {
        if((scale_ = toFloat(v, lineNumber, QString("scale"))) == -1)
            return false;
        else if(scale_ < 1) {
            emit error(QString("Error: scale must be greater than 1 for zoom event in line %1").arg(lineNumber));
            return false;
        }
    }
    else if(param == "color") {
        if(value=="black") color_=cv::Scalar(0, 0, 0);
        else if(value=="white") color_=cv::Scalar(255, 255, 255);
        else if(value=="red") color_=cv::Scalar(0, 0, 255);
        else if(value=="blue") color_=cv::Scalar(255, 0, 0);
        else {
            emit error(QString("Error: couldn't understand color '%1' in line %2. Try black, white, red or blue.").arg(v).arg(lineNumber));
            return false;
        }
    }
    else if(param.replace(" ", "").isEmpty());
    else {
        emit error(QString("Error: couldn't understand '%1' in line %2.").arg(param).arg(lineNumber));
        return false;
    }

    return true;
}

bool EventParser::parseObjectParam(const QString &p, const QString &v, int lineNumber)
{
    QString param = p.toLower().replace(" ", "");
    if(param == "type") {
        if(((objectType_ = v) != "video") && (objectType_ != "image")) {
            emit error(QString("Error: invalid type '%1' in line %2").arg(objectType_).arg(lineNumber));
            return false;
        }
    }
    else if(param == "filename") {
        fileName_ = v;
    }
    else if(param == "id") {
        if((objectId_ = toInt(v, lineNumber, "id")) == -1)
            return false;
    }
    else if(param == "duration") {
        if((duration_ = toInt(v, lineNumber, "duration")) == -1)
            return false;
    }
    else {
        emit error(QString("Error: couldn't understand '%1' in line %2.").arg(p.simplified()).arg(lineNumber));
        return false;
    }

    return true;
}

bool EventParser::createEvent(EventPtr &ev, int lineNumber)
{
    switch(type_) {
    case Event::EVENT_FLIP:
        ev.reset(new FlipEvent(start_, delay_, eventId_, trigCode_));
        ev->appendLog(QString("Flip event added."));
        break;

    case Event::EVENT_FADEIN:
        ev.reset(new FadeInEvent(start_, duration_, delay_, eventId_, trigCode_));
        ev->appendLog(QString("Fade in event added. "));
        break;

    case Event::EVENT_FADEOUT:
        ev.reset(new FadeOutEvent(start_, duration_, delay_, eventId_, trigCode_));
        ev->appendLog(QString("Fade out event added. "));
        break;

    case Event::EVENT_IMAGE:
        if(objectIdOk_) {
            if(!imageObjects_.contains(objectId_)) {
                emit error(QString("Error: couldn't find image object with id %1 in line %2").arg(objectId_).arg(lineNumber));
                return false;
            }
            ev.reset(new ImageEvent(start_, cv::Point2i(x_, y_), imageObjects_[objectId_], delay_, eventId_, trigCode_));
            ev->appendLog(QString("Image event added. "));
        }
        else {
            emit error(QString("Error: image event declared without object id in line %1").arg(lineNumber));
            return false;
        }
        break;

    case Event::EVENT_TEXT:
        ev.reset(new TextEvent(start_, string_, color_, cv::Point2i(x_, y_), delay_, eventId_, trigCode_));
        ev->appendLog(QString("Text event added. "));
        break;

    case Event::EVENT_FREEZE:
        ev.reset(new FreezeEvent(start_, delay_, eventId_, trigCode_));
        ev->appendLog(QString("Freeze event added. "));
        break;

    case Event::EVENT_ROTATE:
        ev.reset(new RotateEvent(start_, angle_, delay_, eventId_, trigCode_));
        ev->appendLog(QString("Rotate event added. "));
        break;

    case Event::EVENT_ZOOM:
        ev.reset(new ZoomEvent(start_, scale_, duration_, delay_, eventId_, trigCode_));
        ev->appendLog(QString("Zoom event added"));
        break;

    case Event::EVENT_DETECT_MOTION:
        ev.reset(new MotionDetectorEvent(start_, delay_, eventId_, trigCode_));
        ev->appendLog(QString("Movement detected"));
        break;

    case Event::EVENT_RECORD:
        if(objectIdOk_) {
            if(!videoObjects_.contains(objectId_)) {
                emit error(QString("Error: couldn't find video object with id %1 in line %2").arg(objectId_).arg(lineNumber));
                return false;
            }
            if(duration_ > videoObjects_[objectId_]->duration_) {
                emit error(QString("Error: record event duration too big for video object in line %1").arg(lineNumber));
                return false;
            }
            ev.reset(new RecordEvent(start_, videoObjects_[objectId_], delay_, duration_, eventId_, trigCode_));
            ev->appendLog(QString("Record event added"));
        }
        else {
            emit error(QString("Error: record event declared without object id in line %1").arg(lineNumber));
            return false;
        }
        break;

    case Event::EVENT_PLAYBACK:
        if(objectIdOk_) {
            if(!videoObjects_.contains(objectId_)) {
                emit error(QString("Error: couldn't find video object with id %1 in line %2").arg(objectId_).arg(lineNumber));
                return false;
            }
            ev.reset(new PlaybackEvent(start_, videoObjects_[objectId_], delay_, duration_, eventId_, trigCode_));
            ev->appendLog(QString("Playback event added."));
        }
        else {
            emit error(QString("Error: playback event declared without object id in line %1").arg(lineNumber));
            return false;
        }
        break;

    default:
        emit error(QString("Error: event declared without a type in line %1").arg(lineNumber));
        return false;

    }

    return true;
}

float EventParser::toFloat(const QString &str, int line, const QString &param) const
{
    bool ok;
    QString string = str.toLower().replace(" ", "");
    float num = string.toFloat(&ok);
    if(!ok) {
        emit error(QString("Error: couldn't convert %1 '%2' in line %3 to floating point").arg(param).arg(str).arg(line));
        return -1;
    }
    if(num < 0) {
        emit error(QString("Error: %1 on line %2 must be a positive floating point").arg(param).arg(line));
        return -1;
    }
    return  num;
}

int EventParser::toInt(const QString &str, int line, const QString &param) const
{
    bool ok;
    QString string = str.toLower().replace(" ", "");
    int num = string.toInt(&ok);
    if(!ok) {
        emit error(QString("Error: Couldn't convert %1 '%2' in line %3 to integer").arg(param).arg(str).arg(line));
        return -1;
    }
    if(num < 0) {
        emit error(QString("Error: %1 on line %2 must be a positive integer").arg(param).arg(line));
        return -1;
    }
    return  num;
}
