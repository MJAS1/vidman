/*
 * camera.cpp
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

#include <iostream>
#include <unistd.h>

#include "config.h"
#include "camera.h"

using namespace std;

Camera::Camera() :
    dc1394Camera_(nullptr), dc1394Context_(nullptr),  camList_(nullptr),
    empty_(true)
{
    // Initialize the camera
    capCam_.open(CV_CAP_FIREWIRE);
    if(!capCam_.isOpened()) {
        cerr << "No cameras found" << endl;
        return;
    }

    dc1394error_t err;
    dc1394Context_ = dc1394_new();
    if(!dc1394Context_) {
        cerr << "Cannot initialize" << endl;
        return;
    }

    err = dc1394_camera_enumerate(dc1394Context_, &camList_);
    if (err != DC1394_SUCCESS) {
        cerr << "Failed to enumerate cameras" << endl;
        return;
    }

    if (camList_->num == 0) {
        cerr << "No cameras found" << endl;
        return;
    }

    // Use the first camera in the list
    dc1394Camera_ = dc1394_camera_new(dc1394Context_, camList_->ids[0].guid);
    if (!dc1394Camera_) {
        cerr << "Failed to initialize camera with guid " << camList_->ids[0].guid
             << endl;
        return;
    }
    cout << "Using camera with GUID " << dc1394Camera_->guid << endl;

    capCam_.set(CV_CAP_PROP_BUFFERSIZE, RingBufSz);
    empty_ = false;
}

Camera::~Camera()
{
    if(camList_)
        dc1394_camera_free_list(camList_);
    if(dc1394Camera_)
        dc1394_camera_free(dc1394Camera_);
    if(dc1394Context_)
        dc1394_free(dc1394Context_);
}

void Camera::setFPS(int fps)
{
    if(!capCam_.set(CV_CAP_PROP_FPS, fps))
        cerr << "Couldn't set framerate" << endl;
}

void Camera::operator >>(cv::Mat& frame)
{
    capCam_ >> frame;
}

bool Camera::empty() const
{
    return empty_;
}

uint32_t Camera::getShutter() const
{
    uint32_t shutter;
    dc1394_get_register(dc1394Camera_, SHUTTER_ADDR, &shutter);
    return shutter;
}

uint32_t Camera::getGain() const
{
    uint32_t gain;
    dc1394_get_register(dc1394Camera_, GAIN_ADDR, &gain);
    return gain;
}

void Camera::setShutter(int newVal)
{
    dc1394error_t err;
    err = dc1394_set_register(dc1394Camera_, SHUTTER_ADDR,
                              newVal + SHUTTER_OFFSET);

    if (err != DC1394_SUCCESS)
        cerr << "Couldn't set shutter register" << endl;
}

void Camera::setGain(int newVal)
{
    dc1394error_t err;
    err = dc1394_set_register(dc1394Camera_, GAIN_ADDR, newVal + GAIN_OFFSET);

    if (err != DC1394_SUCCESS)
        cerr << "Could not set gain register" << endl;
}

void Camera::setUV(int newVal, int vrValue)
{
    dc1394error_t err;

    // UV and VR are located in the same register so both need to be used.
    err = dc1394_set_register(dc1394Camera_, WHITEBALANCE_ADDR,
                              newVal * UV_REG_SHIFT + vrValue
                              + WHITEBALANCE_OFFSET);

    if (err != DC1394_SUCCESS)
        cerr << "Couldn't set white balance register" << endl;
}

void Camera::setVR(int newVal, int uvValue)
{
    dc1394error_t err;

    // UV and VR are located in the same register, so both need to be used.
    err = dc1394_set_register(dc1394Camera_, WHITEBALANCE_ADDR,
                              newVal + UV_REG_SHIFT * uvValue
                              + WHITEBALANCE_OFFSET);

    if (err != DC1394_SUCCESS)
        cerr << "Couldn't set white balance register" << endl;
}

uint32_t Camera::getWhiteBalance() const
{
    uint32_t wb;
    dc1394_get_register(dc1394Camera_, WHITEBALANCE_ADDR, &wb);
    return wb;
}

void Camera::setWhiteBalance(uint32_t wb)
{
    dc1394error_t err;

    err = dc1394_set_register(dc1394Camera_, WHITEBALANCE_ADDR, wb);

    if (err != DC1394_SUCCESS)
        cerr << "Couldn't set white balance register" << endl;
}

void Camera::setExternTrigger(bool on)
{
    dc1394error_t err;
    err = dc1394_external_trigger_set_power(dc1394Camera_,
                                            on ? DC1394_ON : DC1394_OFF);

    if (err != DC1394_SUCCESS)
        cerr << "Couldn't set external trigger." << endl;
}
