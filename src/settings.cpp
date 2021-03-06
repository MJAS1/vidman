/*
 * settings.cpp
 *
 * Original file:
 * Author: Andrey Zhdanov
 * Copyright (C) 2014 BioMag Laboratory, Helsinki University Central Hospital
 *
 * Modifications:
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

#include "settings.h"
#include "config.h"

Settings::Settings() : settings_(ORG_NAME, APP_NAME)
{
    //---------------------------------------------------------------------
    // Video settings
    //

    // JPEG quality
    if(!settings_.contains("video/jpeg_quality")) {
        settings_.setValue("video/jpeg_quality", 80);
        jpgQuality = 80;
    }
    else {
        jpgQuality = settings_.value("video/jpeg_quality").toInt();
    }

    // Use color mode
    if(!settings_.contains("video/color")) {
        settings_.setValue("video/color", true);
        color = true;
    }
    else {
        color = settings_.value("video/color").toBool();
    }

    // Use vsync
    if(!settings_.contains("video/vsync")) {
        settings_.setValue("video/vsync", true);
        vsync = false;
    }
    else {
        vsync = settings_.value("video/vsync").toBool();
    }

    // Flip the frames by default.
    if(!settings_.contains("video/flip")) {
        settings_.setValue("video/flip", true);
        flip = true;
    }
    else {
        flip = settings_.value("video/flip").toBool();
    }

    /*
     * 0: Vertical flip
     * Positive integer e.g. 1: Horizontal flip
     * Negative integer e.g. -1: Horizontal and vertical flip
     */
    if(!settings_.contains("video/flipCode")) {
        settings_.setValue("video/flipCode", 0);
        flipCode = 0;
    }
    else {
        flipCode = settings_.value("video/flipCode").toInt();
    }

    // Add a fixation point to the video
    if(!settings_.contains("video/fixPoint")) {
        settings_.setValue("video/fixPoint", true);
        fixPoint = true;
    }
    else {
        fixPoint = settings_.value("video/fixPoint").toBool();
    }

    // Frames per second
    if(!settings_.contains("video/fps")) {
        settings_.setValue("video/fps", 60);
        fps = 60;
    }
    else {
        fps = settings_.value("video/fps").toInt();
    }

    // Threhsold for movement detection
    if(!settings_.contains("video/movementThreshold")) {
        settings_.setValue("video/movementThreshold", 10);
        movementThreshold = 10;
    }
    else {
        movementThreshold = settings_.value("video/movementThreshold").toInt();
    }

    // Video frame width used for changing aspect ratio
    if(!settings_.contains("video/videoWidth")) {
        settings_.setValue("video/videoWidth", 640);
        videoWidth = 640;
    }
    else {
        videoWidth = settings_.value("video/videoWidth").toInt();
    }

    if(!settings_.contains("video/whiteBalance")) {
        whitebalance = (UV_MAX_VAL*UV_REG_SHIFT + VR_MAX_VAL + WHITEBALANCE_OFFSET) / 2;
        settings_.setValue("video/whiteBalance", whitebalance);
    }
    else {
        whitebalance = settings_.value("video/whiteBalance").toInt();
    }

    if(!settings_.contains("video/shutter")) {
        shutter = (SHUTTER_MAX_VAL - SHUTTER_MIN_VAL) / 10;
        settings_.setValue("video/shutter", shutter);
    }
    else {
        shutter = settings_.value("video/shutter").toInt();
    }

    if(!settings_.contains("video/gain")) {
        gain = (GAIN_MAX_VAL - GAIN_MIN_VAL) / 10;
        settings_.setValue("video/gain", gain);
    }
    else {
        gain = settings_.value("video/gain").toInt();
    }

    //---------------------------------------------------------------------
    // Misc settings_
    //

    // Data storage folder
    if(!settings_.contains("misc/data_storage_path")) {
        settings_.setValue("misc/data_storage_path", "../videodat");
        sprintf(storagePath, "../viddeodat");
    }
    else {
        sprintf(storagePath, settings_.value("misc/data_storage_path").toString().toLocal8Bit().data());
    }

    // Parallel port address
    if(!settings_.contains("misc/parallel_port_address")) {
        settings_.setValue("misc/parallel_port_address", 0x3010);
        printerPortAddr = 0x3010;
    }
    else {
        printerPortAddr = settings_.value("misc/parallel_port_address").toInt();
    }

    /*
     * Default trigger codes for frames. The trigger code will toggle between,
     * defaultTrig1 and defaultTrig2
     */
    if(!settings_.contains("misc/default_trigger1")) {
        settings_.setValue("misc/default_trigger1", 1);
        defaultTrig1 = 1;
    }
    else {
        defaultTrig1 = settings_.value("misc/default_trigger1").toInt();
    }

    if(!settings_.contains("misc/default_trigger2")) {
        settings_.setValue("misc/default_trigger2", 2);
        defaultTrig2 = 1;
    }
    else {
        defaultTrig2 = settings_.value("misc/default_trigger2").toInt();
    }
}

void Settings::setValue(const QString &key, const QVariant &value)
{
    settings_.setValue(key, value);
}

