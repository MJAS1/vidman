/*
 * camera.h
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

#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>
#include <dc1394/dc1394.h>

/*!
 * This class is used to control a FireWire video camera. It uses dc1394
 * library to access the camera's register and OpenCV to acquire frames.
 */
class Camera
{
public:
    Camera();
    ~Camera();

    bool empty() const;

    uint32_t getShutter() const;
    uint32_t getGain() const;
    uint32_t getWhiteBalance() const;

    void setShutter(int newVal);
    void setGain(int newVal);
    void setUV(int newVal, int vrValue);
    void setVR(int newVal, int uvValue);
    void setWhiteBalance(uint32_t wb);
    void setFPS(int fps);

    /*
     * Allows the camera to be controlled by an external trigger source
     * (e.g., a signal generator).
     */
    void setExternTrigger(bool on);

    // Grab frame.
    void operator>>(cv::Mat& frame);

private:
    /*
     * Camera frame buffersize of 1 lowers the maximum framerate for some reason
     * so use at least 2.
     */
    static const int RingBufSz = 2;

    Camera(const Camera&);
    Camera& operator=(const Camera&);

    cv::VideoCapture        capCam_;
    dc1394camera_t*         dc1394Camera_;
    dc1394_t*               dc1394Context_;
    dc1394camera_list_t*    camList_;

    bool                    empty_;
};

#endif // CAMERA_H
