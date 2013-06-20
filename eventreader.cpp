#include <iostream>
#include <QMessageBox>
#include "eventreader.h"

EventReader::EventReader()
{
}

bool EventReader::loadEvents(const QStringList &strList, EventContainer *events)
{
    events->clear();
    for(int i = 0; i < strList.size(); i++)
    {
        QStringList split = strList[i].split(':');
        QString str = split[0].toLower().replace(" ", "").simplified();

        if(str.startsWith("#comment"))
        {
            while(!str.contains("#endcomment"))
            {
                i++;
                if(i < strList.size())
                    str = strList[i].toLower().replace(" ", "");
                else
                    break;

            }
            if(i >= strList.size())
                break;

            continue;
        }

        if(str == "#event")
        {
            if(!readEvent(split[1], events, i+1))
                return false;
        }
        else if(str == "#imageobject")
        {
            if(!readImageObject(split[1], i+1))
                return false;
        }
        else if(str == "#removeevent")
        {
            if(!readRemoveEvent(split[1], events, i+1))
                return false;
        }
        else if(str[0] == '#' || split[0].simplified() == "");
        else
        {
            errorMsg(QString("Couldn't understand '%1' in line %2.").arg(split[0]).arg(i+1));
            return false;
        }
    }

    return true;
}

bool EventReader::readEvent(const QString &str, EventContainer *events, int lineNumber)
{
    QStringList strList = str.split(',');

    float start = 0, duration = 0;
    int x = 0, y = 0, imageId = 0, eventId = -1, angle = 0, delay=0;
    bool typeOk = false, imageIdOk = false;
    QString text;
    EventType type;

    for(int i = 0; i < strList.size(); i++)
    {
        QStringList split = strList[i].split('=');
        QString param = split[0].toLower().replace(" ", "");
        QString value = split[1].toLower().replace(" ", "");

        if(param == "type")
        {
            typeOk = true;
            if(value == "flip") type = EVENT_FLIP;
            else if(value == "fadein") type = EVENT_FADEIN;
            else if(value == "fadeout") type = EVENT_FADEOUT;
            else if(value == "image") type = EVENT_IMAGE;
            else if(value == "text") type = EVENT_TEXT;
            else if(value == "freeze") type = EVENT_FREEZE;
            else if(value == "rotate") type = EVENT_ROTATE;
            else if(value == "remove") type = EVENT_REMOVE;
            else
            {
                errorMsg(QString("Couldn't understand type '%1' in line %2.").arg(split[1]).arg(lineNumber));
                return false;
            }
        }
        else if(param == "start")
        {
            if((start = toFloat(split[1], lineNumber, QString("start time"))) == -1)
                return false;
        }
        else if(param == "duration")
        {
            if((duration = toFloat(split[1], lineNumber, QString("duration"))) == -1)
                return false;
        }
        else if(param == "x")
        {
            if((x = toInt(split[1], lineNumber, QString("x-coordinate"))) == -1)
                return false;
        }
        else if(param == "y")
        {
            if((y = toInt(split[1], lineNumber, QString("y-coordinate"))) == -1)
                return false;
        }
        else if(param == "imageid")
        {
            if((imageId = toInt(split[1], lineNumber, QString("image ID"))) == -1)
                return false;

            if(!imageContainer.contains(imageId))
            {
                errorMsg(QString("Couldn't find image object with id %1 in line %2").arg(imageId).arg(lineNumber));
                return false;
            }
            imageIdOk = true;
        }
        else if(param == "text")
        {
            text=split[1];
        }
        else if(param == "angle")
        {
            if((angle = toInt(split[1], lineNumber, QString("angle"))) == -1)
                return false;
        }
        else if(param == "id")
        {
            if((eventId = toInt(split[1], lineNumber, QString("id"))) == -1)
                return false;
        }
        else if(param == "delay")
        {
            if((delay = toFloat(split[1], lineNumber, QString("delay"))) == -1)
                return false;
        }
        else if(param.replace(" ", "").isEmpty());
        else
        {
            errorMsg(QString("Couldn't understand '%1' in line %2.").arg(param).arg(lineNumber));
            return false;
        }

    }

    if(typeOk)
    {
        Event* ev;
        if(type == EVENT_FLIP)
        {
            ev = new FlipEvent(start, delay, eventId);
        }
        else if(type == EVENT_FADEIN)
        {
            ev = new FadeInEvent(start, duration, delay);
        }
        else if(type == EVENT_FADEOUT)
        {
            ev = new FadeOutEvent(start, duration, delay);
        }
        else if(type == EVENT_IMAGE)
        {
            if(imageIdOk)
                ev = new ImageEvent(start, cv::Point2i(x, y), imageContainer[imageId], delay, eventId);
            else
            {
                errorMsg(QString("Image event declared without object id in line %1").arg(lineNumber));
                return false;
            }
        }
        else if(type == EVENT_TEXT)
        {
            ev = new TextEvent(start, text, cv::Point2i(x, y), delay, eventId);
        }
        else if(type == EVENT_FREEZE)
        {
            ev = new FreezeEvent(start, delay, eventId);
        }
        else if(type == EVENT_ROTATE)
        {
            ev = new RotateEvent(start, angle, delay, eventId);
        }
        events->push_back(ev);
    }
    else
    {
        errorMsg(QString("Event declared without type in line %1").arg(lineNumber));
        return false;
    }

    return true;
}

bool EventReader::readImageObject(const QString &str, int lineNumber)
{
    QStringList strList = str.split(',');

    QString filename("");
    int id = 0;

    for(int i = 0; i < strList.size(); i++)
    {
        QStringList split = strList[i].split('=');
        QString param = split[0].toLower().replace(" ", "");
        QString value = split[1];

        if(param == "filename")
        {
            filename = value;
        }

        else if(param == "id")
        {
            if((id = toInt(value, lineNumber, "id")) == -1)
                return false;
        }
        else
        {
            errorMsg(QString("Couldn't understand '%1' in line %2.").arg(split[0].simplified()).arg(lineNumber));
            return false;
        }
    }

    if(!imageContainer.addImage(id, filename))
    {
        errorMsg(QString("Couldn't load file %1.").arg(filename));
        return false;
    }

    return true;
}


bool EventReader::readRemoveEvent(const QString &str, EventContainer *events, int lineNumber)
{
    QStringList strList = str.split(',');

    int id;
    float start = 0, delay=0;
    bool idOk = false;

    for(int i = 0; i < strList.size(); i++)
    {
        QStringList split = strList[i].split("=");
        QString param = split[0].toLower().replace(" ", "");
        QString value = split[1];

        if(param == "id")
        {
            if((id = toInt(value, lineNumber, "id")) == -1)
                return false;

            idOk = true;
        }
        else if(param == "start")
        {
            if((start = toFloat(value, lineNumber, "start time")) == -1)
                return false;
        }
        else
        {
            errorMsg(QString("Couldn't understand '%1' in line %2.").arg(split[0].simplified()).arg(lineNumber));
            return false;
        }
    }

    if(idOk)
    {
        Event* ev = new RemoveEvent(start, delay, id);
        events->push_back(ev);
    }
    else
    {
        errorMsg(QString("Remove event declared without id in line %1").arg(lineNumber));
    }

    return true;

}

void EventReader::errorMsg(const QString &message) const
{
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.exec();
}

float EventReader::toFloat(const QString &str, const int line, const QString &param) const
{
    bool ok;
    QString string = str.toLower().replace(" ", "");
    float num = string.toFloat(&ok);
    if(!ok)
    {
        errorMsg(QString("Couldn't convert %1 '%2' in line %3 to floating point").arg(param).arg(str).arg(line));
        return -1;
    }
    if(num < 0)
    {
        errorMsg(QString("%1 on line %2 must be a positive floating point").arg(param).arg(line));
        return -1;
    }
    return  num;
}

int EventReader::toInt(const QString &str, const int line, const QString &param) const
{
    bool ok;
    QString string = str.toLower().replace(" ", "");
    float num = string.toInt(&ok);
    if(!ok)
    {
        errorMsg(QString("Couldn't convert %1 '%2' in line %3 to integer").arg(param).arg(str).arg(line));
        return -1;
    }
    if(num < 0)
    {
        errorMsg(QString("%1 on line %2 must be a positive integer").arg(param).arg(line));
        return -1;
    }
    return  num;
}
