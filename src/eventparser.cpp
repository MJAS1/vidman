#include <iostream>
#include <QStringList>
#include <sys/ioctl.h>
#include "mainwindow.h"
#include "eventparser.h"
#include "eventcontainer.h"

using std::move;

bool EventParser::loadEvents(const QStringList &strList, EventContainer& events,
                             MainWindow* window)
{
    imageObjects_.clear();
    videoObjects_.clear();
    events.clear();

    //Start parsing the events, line by line
    for(int i = 0; i < strList.size(); i++)
    {
        currentLine_ = i+1;

        /* The string should be of the form "Event: type=.., start=.., etc.."
         * Split from ':', remove whitespaces and convert to lower case. */
        QStringList split = strList[i].split(':');
        QString str = split[0].toLower().replace(" ", "").simplified();

        //Ignore comments
        if(str[0] == '#' || split[0].simplified() == "");
        else if(split.size() > 1) {
            if(str == "event") {
                if(!parseEvent(split[1], events))
                    return false;
            }
            else if(str == "object") {
                if(!parseObject(split[1]))
                    return false;
            }
            else if(str == "delete") {
                if(!parseDelEvent(split[1], events))
                    return false;
            }
            else {
                emit error(QString("Error: couldn't understand '%1' in line %2.")
                           .arg(split[0]).arg(currentLine_));
                return false;
            }
        }
        else if(str == "pause") {
            EventPtr ev(new PauseEvent);
            connect(ev.get(), SIGNAL(triggered(uint8_t,QString)),
                    window, SLOT(pause()));
            events.append(move(ev));
        }
        else if(!str.isEmpty()) {
            emit error(QString("Error: couldn't understand line %1.").arg(currentLine_));
            return false;
        };
    }
    return true;
}

bool EventParser::parseEvent(const QString &str, EventContainer& events) const
{
    /* Split the string to get a list of strings of the format param=val e.g.,
     * {"type=...", "start=...", etc..}. */
    QStringList strList = str.split(',');
    EventAttributes attr;

    //Fill all the parameters required to create an event
    for(int i = 0; i < strList.size(); i++) {
        if(strList[i].contains("=")) {
            QStringList split = strList[i].split('=');
            if(!parseEventParam(split[0], split[1], attr))
                return false;
        }
    }

    //Create the new event with the acquired parameters
    EventPtr ev = createEvent(attr);
    if(!ev)
        return false;
    events.append(move(ev));
    return true;
}

bool EventParser::parseDelEvent(const QString &str, EventContainer& events) const
{
    QStringList strList = str.split(',');
    EventAttributes attr;

    for(int i = 0; i < strList.size(); i++) {
        if(strList[i].contains("=")) {
            QStringList split = strList[i].split("=");
            QString param = split[0];
            QString value = split[1];
            if(!parseEventParam(param, value, attr))
                return false;
        }
    }

    EventPtr ev = createDelEvent(attr);
    if(!ev)
        return false;
    events.append(move(ev));
    return true;
}

bool EventParser::parseObject(const QString &str)
{
    QStringList strList = str.split(',');
    EventAttributes attr;

    for(int i = 0; i < strList.size(); i++) {
        if(strList[i].contains("=")) {
            QStringList split = strList[i].split('=');
            QString param = split[0];
            QString value = split[1];

            if(!parseObjectParam(param, value, attr))
                return false;
        }
    }

    if(attr.objectType_ == "image") {
        cv::Mat file(cv::imread(attr.fileName_.toStdString(),
                                CV_LOAD_IMAGE_UNCHANGED));
        if(file.empty()) {
            emit error(QString("Error: couldn't load image file '%1'.")
                       .arg(attr.fileName_));
            return false;
        }
        imageObjects_[attr.objectId_] = file;
    }
    else if(attr.objectType_ == "video"){
        shared_ptr<VideoObject> videoObject(new VideoObject);
        videoObject->duration_ = attr.duration_;

        /* Reserve enough memory to hold all the frames. This is necessary to
         * make sure that large blocks of memory don't need to be reallocated
         * while recording. */
        videoObject->frames_.reserve(attr.duration_/1000*settings_.fps + 10);
        videoObjects_.insert(attr.objectId_, videoObject);
    }
    else if(attr.objectType_ == "") {
        emit error(QString("Error: no type declared for object in line %1")
                   .arg(currentLine_));
        return false;
    }
    else {
        emit error(QString("Error: invalid type '%1' for object in line %1")
                   .arg(attr.objectType_).arg(currentLine_));
        return false;
    }
    return true;
}

bool EventParser::parseEventParam(const QString &p, const QString &v,
                                  EventAttributes &attr) const
{
    QString param = p.toLower().replace(" ", "");
    QString value = v.toLower().replace(" ", "");

    if(param == "type") {
        if(value == "flip") attr.type_ = Event::EVENT_FLIP;
        else if(value == "fade_in") attr.type_ = Event::EVENT_FADEIN;
        else if(value == "fade_out") attr.type_ = Event::EVENT_FADEOUT;
        else if(value == "image") attr.type_ = Event::EVENT_IMAGE;
        else if(value == "text") attr.type_ = Event::EVENT_TEXT;
        else if(value == "freeze") attr.type_ = Event::EVENT_FREEZE;
        else if(value == "rotate") attr.type_ = Event::EVENT_ROTATE;
        else if(value == "detect_motion") attr.type_ = Event::EVENT_DETECT_MOTION;
        else if(value == "zoom") attr.type_ = Event::EVENT_ZOOM;
        else if(value == "record") attr.type_ = Event::EVENT_RECORD;
        else if(value == "playback") attr.type_ = Event::EVENT_PLAYBACK;
        else {
            emit error(QString("Error: invalid type '%1' in line %2.")
                       .arg(v).arg(currentLine_));
            return false;
        }
    }
    else if(param == "start") {
        if((attr.start_ = toInt(v, QString("start time"))) == -1)
            return false;
    }
    else if(param == "duration") {
        if((attr.duration_ = toInt(v, QString("duration"))) == -1)
            return false;
    }
    else if(param == "x") {
        if((attr.x_ = toInt(v, QString("x-coordinate"))) == -1)
            return false;
    }
    else if(param == "y") {
        if((attr.y_ = toInt(v, QString("y-coordinate"))) == -1)
            return false;
    }
    else if(param == "object_id") {
        if((attr.objectId_ = toInt(v, QString("objectID"))) == -1)
            return false;
        attr.objectIdOk_ = true;
    }
    else if(param == "string") {
        attr.string_=v;
    }
    else if(param == "angle") {
        if((attr.angle_ = toInt(v, QString("angle"))) == -1)
            return false;
    }
    else if(param == "id") {
        if((attr.eventId_ = toInt(v, QString("id"))) == -1)
            return false;
    }
    else if(param == "delay") {
        if((attr.delay_ = toInt(v, QString("delay"))) == -1)
            return false;
    }
    else if(param == "trig_code") {
        if(value == "dtr") attr.trigCode_ = TIOCM_DTR;
        else if(value == "rts") attr.trigCode_ = TIOCM_RTS;
        else if((attr.trigCode_ = toInt(v, QString("trigcode"))) == -1)
            return false;
        if(attr.trigCode_ > 255) {
            emit error(QString("Error: trigcode must be between 0 and 255 "
                               "in line %1.").arg(currentLine_));
            return false;
        }
    }
    else if(param == "scale") {
        if((attr.scale_ = toFloat(v, QString("scale"))) == -1)
            return false;
        else if(attr.scale_ < 1) {
            emit error(QString("Error: scale must be greater than 1 for zoom "
                               "event in line %1").arg(currentLine_));
            return false;
        }
    }
    else if(param == "color") {
        if(value=="black") attr.color_=cv::Scalar(0, 0, 0);
        else if(value=="white") attr.color_=cv::Scalar(255, 255, 255);
        else if(value=="red") attr.color_=cv::Scalar(0, 0, 255);
        else if(value=="blue") attr.color_=cv::Scalar(255, 0, 0);
        else {
            emit error(QString("Error: couldn't understand color '%1' in"
                               "line %2. Try black, white, red or blue.")
                       .arg(v).arg(currentLine_));
            return false;
        }
    }
    else if(param == "success_code") {
        if((attr.successCode_ = toInt(v, QString("successcode"))) == -1)
            return false;
        if(attr.successCode_ > 255) {
            emit error(QString("Error: succescode must be between 0 and 255 "
                               "in line %1.").arg(currentLine_));
            return false;
        }
    }
    else if(param == "fail_code") {
        if((attr.failCode_ = toInt(v, QString("failcode"))) == -1)
            return false;
        if(attr.failCode_ > 255) {
            emit error(QString("Error: failcode must be between 0 and 255 "
                               "in line %1.").arg(currentLine_));
            return false;
        }
    }
    else if(param == "target") {
        if((attr.target_ = toInt(v, QString("target"))) == -1)
            return false;
    }
    else if(param == "tolerance") {
        if((attr.tolerance_ = toInt(v, QString("tolerance"))) == -1)
            return false;
    }
    else if(param == "axis") {
        if(v == "x") attr.axis_ = 0;
        else if(v == "y") attr.axis_ = 1;
        else if(v == "both") attr.axis_ = -1;
        else {
            emit error(QString("Error: axis must be 'x', 'y' or 'both'"
                                "in line %2.").arg(currentLine_));
            return false;
        }
    }
    else if(!param.replace(" ", "").isEmpty()) {
        emit error(QString("Error: couldn't understand '%1' in line %2.")
                   .arg(param).arg(currentLine_));
        return false;
    }

    return true;
}

bool EventParser::parseObjectParam(const QString &p, const QString &v,
                                   EventAttributes &attr) const
{
    QString param = p.toLower().replace(" ", "");
    if(param == "type") {
        if(((attr.objectType_ = v) != "video") && (attr.objectType_ != "image")) {
            emit error(QString("Error: invalid type '%1' in line %2")
                       .arg(attr.objectType_).arg(currentLine_));
            return false;
        }
    }
    else if(param == "filename") {
        attr.fileName_ = v;
    }
    else if(param == "id") {
        if((attr.objectId_ = toInt(v, "id")) == -1)
            return false;
    }
    else if(param == "duration") {
        if((attr.duration_ = toInt(v, "duration")) == -1)
            return false;
    }
    else {
        emit error(QString("Error: couldn't understand '%1' in line %2.")
                   .arg(p.simplified()).arg(currentLine_));
        return false;
    }

    return true;
}

EventPtr EventParser::createEvent(const EventAttributes &attr) const
{
    EventPtr ev;
    QString errStr;

    switch(attr.type_) {
    case Event::EVENT_FLIP:
        ev.reset(new FlipEvent(attr.start_, attr.axis_, attr.delay_,
                               attr.eventId_, attr.trigCode_));
        ev->appendLog(QString("Flip event added."));
        break;

    case Event::EVENT_FADEIN:
        ev.reset(new FadeInEvent(attr.start_, attr.duration_, attr.delay_,
                                 attr.eventId_, attr.trigCode_));
        ev->appendLog(QString("Fade in event added. "));
        break;

    case Event::EVENT_FADEOUT:
        ev.reset(new FadeOutEvent(attr.start_, attr.duration_, attr.delay_,
                                  attr.eventId_, attr.trigCode_));
        ev->appendLog(QString("Fade out event added. "));
        break;

    case Event::EVENT_IMAGE:
        if(attr.objectIdOk_) {
            if(!imageObjects_.contains(attr.objectId_)) {
                errStr = QString("Error: couldn't find image object with id %1"
                                 "in line %2").arg(attr.objectId_).arg(currentLine_);
            }
            else {
                cv::Point2i p(attr.x_, attr.y_);
                cv::Mat img(imageObjects_[attr.objectId_]);
                ev.reset(new ImageEvent(attr.start_, p, img, attr.delay_,
                                        attr.eventId_, attr.trigCode_));
                ev->appendLog(QString("Image event added. "));
            }
        }
        else {
            errStr = QString("Error: image event declared without object id in"
                             "line %1").arg(currentLine_);
        }
        break;

    case Event::EVENT_TEXT:
        {
        cv::Point2i p(attr.x_, attr.y_);
        ev.reset(new TextEvent(attr.start_, attr.string_, attr.color_, p,
                               attr.delay_, attr.eventId_, attr.trigCode_));
        ev->appendLog(QString("Text event added. "));
        }
        break;

    case Event::EVENT_FREEZE:
        ev.reset(new FreezeEvent(attr.start_, attr.delay_, attr.eventId_,
                                 attr.trigCode_));
        ev->appendLog(QString("Freeze event added. "));
        break;

    case Event::EVENT_ROTATE:
        ev.reset(new RotateEvent(attr.start_, attr.angle_, attr.delay_,
                                 attr.eventId_, attr.trigCode_));
        ev->appendLog(QString("Rotate event added. "));
        break;

    case Event::EVENT_ZOOM:
        ev.reset(new ZoomEvent(attr.start_, attr.scale_, attr.duration_,
                               attr.delay_, attr.eventId_, attr.trigCode_));
        ev->appendLog(QString("Zoom event added"));
        break;

    case Event::EVENT_DETECT_MOTION:
        ev.reset(new MotionDetectorEvent(attr.start_, attr.target_,  attr.tolerance_, attr.delay_,
                                         attr.eventId_, attr.trigCode_, attr.successCode_, attr.failCode_));
        ev->appendLog(QString("Movement detected"));
        break;

    case Event::EVENT_RECORD:
        if(attr.objectIdOk_) {
            if(!videoObjects_.contains(attr.objectId_)) {
                errStr = QString("Error: couldn't find video object with id %1"
                                 "in line %2").arg(attr.objectId_).arg(currentLine_);
            }
            else if(attr.duration_ > videoObjects_[attr.objectId_]->duration_) {
                errStr = QString("Error: record event duration too big for"
                                 "video object in line %1").arg(currentLine_);
            }
            else {
                ev.reset(new RecordEvent(attr.start_, videoObjects_[attr.objectId_], attr.delay_,
                                         attr.duration_, attr.eventId_, attr.trigCode_));
                ev->appendLog(QString("Record event added"));
            }
        }
        else {
            errStr = QString("Error: record event declared without object id in"
                             "line %1").arg(currentLine_);
        }
        break;

    case Event::EVENT_PLAYBACK:
        if(attr.objectIdOk_) {
            if(!videoObjects_.contains(attr.objectId_)) {
                errStr = QString("Error: couldn't find video object with id %1"
                                 "in line %2").arg(attr.objectId_).arg(currentLine_);
            }
            else {
                ev.reset(new PlaybackEvent(attr.start_, videoObjects_[attr.objectId_], attr.delay_,
                                           attr.duration_, attr.eventId_, attr.trigCode_));
                ev->appendLog(QString("Playback event added."));
            }
        }
        else {
            errStr = QString("Error: playback event declared without object id"
                             "in line %1").arg(currentLine_);
        }
        break;

    default:
        errStr = QString("Error: event declared without a type in line %1").arg(currentLine_);
    }

    if(!errStr.isEmpty())
        emit error(errStr);
    return ev;
}

EventPtr EventParser::createDelEvent(EventAttributes &attr) const
{
    EventPtr ev;

    if(attr.eventId_ > -1 && attr.type_) {
        emit error(QString("Error: remove event declared with id and type in"
                           "line %1.").arg(currentLine_));
    }
    else if(attr.eventId_ > -1) {
        ev.reset(new DelEvent(attr.start_, attr.delay_, attr.eventId_,
                                 attr.trigCode_));
        ev->appendLog(QString("Event ID %1 removed. ").arg(attr.eventId_));
    }
    else {
        if(attr.type_) {
            ev.reset(new DelEvent(attr.start_, attr.delay_, attr.type_,
                                  attr.trigCode_));
            switch(attr.type_) {
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
            case Event::EVENT_DETECT_MOTION:
                ev->appendLog(QString("Detect motion event removed."));
                break;
            default:
                break;
            }
        }
        else {
            emit error(QString("Error: remove event declared without id or type"
                               "in line %1").arg(currentLine_));
        }
    }

    return ev;
}

float EventParser::toFloat(const QString &str, const QString &param) const
{
    bool ok;
    QString string = str.toLower().replace(" ", "");
    float num = string.toFloat(&ok);
    if(!ok) {
        emit error(QString("Error: couldn't convert %1 '%2' in line %3 to "
                           "floating point")
                   .arg(param).arg(str).arg(currentLine_));
        return -1;
    }
    if(num < 0) {
        emit error(QString("Error: %1 on line %2 must be a positive floating "
                           "point").arg(param).arg(currentLine_));
        return -1;
    }
    return  num;
}

int EventParser::toInt(const QString &str, const QString &param) const
{
    bool ok;
    QString string = str.toLower().replace(" ", "");
    int num = string.toInt(&ok);
    if(!ok) {
        emit error(QString("Error: Couldn't convert %1 '%2' in line %3 to "
                           "integer").arg(param).arg(str).arg(currentLine_));
        return -1;
    }
    if(num < 0) {
        emit error(QString("Error: %1 on line %2 must be a positive integer")
                   .arg(param).arg(currentLine_));
        return -1;
    }
    return  num;
}
