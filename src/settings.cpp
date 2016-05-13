/*
 * settings.cpp
 *
 *  Created on: Mar 7, 2011
 *      Author: andrey
 */
#include <QSettings>

#include "settings.h"
#include "config.h"

Settings::Settings()
{
    QSettings settings(ORG_NAME, APP_NAME);


    //---------------------------------------------------------------------
    // Video settings
    //

    // JPEG quality
    if(!settings.contains("video/jpeg_quality"))
    {
        settings.setValue("video/jpeg_quality", 80);
        jpgQuality = 80;
    }
    else
    {
        jpgQuality = settings.value("video/jpeg_quality").toInt();
    }

    // Use color mode
    if(!settings.contains("video/color"))
    {
        settings.setValue("video/color", true);
        color = true;
    }
    else
    {
        color = settings.value("video/color").toBool();
    }

    //Use vsync
    if(!settings.contains("video/vsync"))
    {
        settings.setValue("video/vsync", true);
        vsync = false;
    }
    else
    {
        vsync = settings.value("video/vsync").toBool();
    }

    //Flip the video image on default
    if(!settings.contains("video/flip"))
    {
        settings.setValue("video/flip", true);
        flip = true;
    }
    else
    {
        flip = settings.value("video/flip").toBool();
    }

    //Turn the video image upside down on default
    if(!settings.contains("video/rotate"))
    {
        settings.setValue("video/rotate", true);
        turnAround = true;
    }
    else
    {
        turnAround = settings.value("video/rotate").toBool();
    }

    //Add a fixation point to the video
    if(!settings.contains("video/fixPoint"))
    {
        settings.setValue("video/fixPoint", true);
        fixPoint = true;
    }
    else
    {
        fixPoint = settings.value("video/fixPoint").toBool();
    }

    //Frames per second
    if(!settings.contains("video/fps"))
    {
        settings.setValue("video/fps", 60);
        fps = 30;
    }
    else
    {
        fps = settings.value("video/fps").toInt();
    }

    //Sensitivity of movement detection
    if(!settings.contains("video/movementSensitivity"))
    {
        settings.setValue("video/movementSensitivity", 10);
        movementSensitivity = 10;
    }
    else
    {
        movementSensitivity = settings.value("video/movementSensitivity").toInt();
    }

    //---------------------------------------------------------------------
    // Misc settings
    //

    // Data storage folder
    if(!settings.contains("misc/data_storage_path"))
    {
        settings.setValue("misc/data_storage_path", "../videodat");
        sprintf(storagePath, "../viddeodat");
    }
    else
    {
        sprintf(storagePath, settings.value("misc/data_storage_path").toString().toLocal8Bit().data());
    }

    //Parallel port address
    if(!settings.contains("misc/parallel_port_address"))
    {
        settings.setValue("misc/parallel_port_address", 0x3010);
        printerPortAddr = 0x3010;
    }
    else
    {
        printerPortAddr = settings.value("misc/parallel_port_address").toInt();
    }
}

