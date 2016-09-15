/*
 * settings_.cpp
 *
 *  Created on: Mar 7, 2011
 *      Author: andrey
 */
#include "settings.h"
#include "config.h"

Settings::Settings() : settings_(ORG_NAME, APP_NAME)
{
    //---------------------------------------------------------------------
    // Video settings
    //

    // JPEG quality
    if(!settings_.contains("video/jpeg_quality"))
    {
        settings_.setValue("video/jpeg_quality", 80);
        jpgQuality = 80;
    }
    else
    {
        jpgQuality = settings_.value("video/jpeg_quality").toInt();
    }

    // Use color mode
    if(!settings_.contains("video/color"))
    {
        settings_.setValue("video/color", true);
        color = true;
    }
    else
    {
        color = settings_.value("video/color").toBool();
    }

    //Use vsync
    if(!settings_.contains("video/vsync"))
    {
        settings_.setValue("video/vsync", true);
        vsync = false;
    }
    else
    {
        vsync = settings_.value("video/vsync").toBool();
    }

    //Flip the video image on default
    if(!settings_.contains("video/flip"))
    {
        settings_.setValue("video/flip", true);
        flip = true;
    }
    else
    {
        flip = settings_.value("video/flip").toBool();
    }

    //Turn the video image upside down on default
    if(!settings_.contains("video/rotate"))
    {
        settings_.setValue("video/rotate", true);
        turnAround = true;
    }
    else
    {
        turnAround = settings_.value("video/rotate").toBool();
    }

    //Add a fixation point to the video
    if(!settings_.contains("video/fixPoint"))
    {
        settings_.setValue("video/fixPoint", true);
        fixPoint = true;
    }
    else
    {
        fixPoint = settings_.value("video/fixPoint").toBool();
    }

    //Frames per second
    if(!settings_.contains("video/fps"))
    {
        settings_.setValue("video/fps", 60);
        fps = 30;
    }
    else
    {
        fps = settings_.value("video/fps").toInt();
    }

    //Sensitivity of movement detection
    if(!settings_.contains("video/movementThreshold"))
    {
        settings_.setValue("video/movementThreshold", 10);
        movementThreshold = 10;
    }
    else
    {
        movementThreshold = settings_.value("video/movementThreshold").toInt();
    }

    //Video frame width used for changing aspect ratio
    if(!settings_.contains("video/videoWidth"))
    {
        settings_.setValue("video/videoWidth", 640);
        videoWidth = 640;
    }
    else
    {
        videoWidth = settings_.value("video/videoWidth").toInt();
    }

    if(!settings_.contains("video/whiteBalance"))
    {
        whitebalance = (UV_MAX_VAL*UV_REG_SHIFT + VR_MAX_VAL + WHITEBALANCE_OFFSET) / 2;
        settings_.setValue("video/whiteBalance", whitebalance);
    }
    else
    {
        whitebalance = settings_.value("video/whiteBalance").toInt();
    }

    //---------------------------------------------------------------------
    // Misc settings_
    //

    // Data storage folder
    if(!settings_.contains("misc/data_storage_path"))
    {
        settings_.setValue("misc/data_storage_path", "../videodat");
        sprintf(storagePath, "../viddeodat");
    }
    else
    {
        sprintf(storagePath, settings_.value("misc/data_storage_path").toString().toLocal8Bit().data());
    }

    //Parallel port address
    if(!settings_.contains("misc/parallel_port_address"))
    {
        settings_.setValue("misc/parallel_port_address", 0x3010);
        printerPortAddr = 0x3010;
    }
    else
    {
        printerPortAddr = settings_.value("misc/parallel_port_address").toInt();
    }
}

void Settings::setValue(const QString &key, const QVariant &value)
{
    settings_.setValue(key, value);
}

