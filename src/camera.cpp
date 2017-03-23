#include <iostream>
#include <QDebug>
#include <unistd.h>
#include "config.h"
#include "camera.h"

using namespace std;

Camera::Camera() : empty_(true)
{
    //Initialize camera
    capCam_.open(300);
    if(!capCam_.isOpened()) {
        cerr << "No cameras found" << endl;
        return;
    }

    dc1394camera_list_t*	camList;
    dc1394error_t			err;

    dc1394Context_ = dc1394_new();
    if(!dc1394Context_) {
        cerr << "Cannot initialize!" << endl;
        return;
    }

    err = dc1394_camera_enumerate(dc1394Context_, &camList);
    if (err != DC1394_SUCCESS) {
        cerr << "Failed to enumerate cameras" << endl;
        return;
    }

    if (camList->num == 0) {
        cerr << "No cameras found" << endl;
        return;
    }

    // use the first camera in the list
    dc1394camera_ = dc1394_camera_new(dc1394Context_, camList->ids[0].guid);
    if (!dc1394camera_) {
        cerr << "Failed to initialize camera with guid " << camList->ids[0].guid << endl;
        return;
    }
    cout << "Using camera with GUID " << dc1394camera_->guid << endl;

    dc1394_camera_free_list(camList);
    empty_ = false;
}

Camera::~Camera()
{
    free(dc1394Context_);
    free(dc1394camera_);
}

void Camera::setFPS(int fps)
{
    if(!capCam_.set(CV_CAP_PROP_FPS, fps)) {
        cerr << "Could not set framerate" << endl;
        abort();
    }
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
    dc1394_get_register(dc1394camera_, SHUTTER_ADDR, &shutter);
    return shutter;
}

uint32_t Camera::getGain() const
{
    uint32_t gain;
    dc1394_get_register(dc1394camera_, GAIN_ADDR, &gain);
    return gain;
}

void Camera::setShutter(int newVal)
{
    dc1394error_t	err;
    err = dc1394_set_register(dc1394camera_, SHUTTER_ADDR, newVal + SHUTTER_OFFSET);

    if (err != DC1394_SUCCESS)
        cerr << "Could not set shutter register" << endl;

}

void Camera::setGain(int newVal)
{
    dc1394error_t	err;
    err = dc1394_set_register(dc1394camera_, GAIN_ADDR, newVal + GAIN_OFFSET);

    if (err != DC1394_SUCCESS)
        cerr << "Could not set gain register" << endl;
}

void Camera::setUV(int newVal, int vrValue)
{
    dc1394error_t	err;

    // Since UV and VR live in the same register, we need to take care of both
    err = dc1394_set_register(dc1394camera_, WHITEBALANCE_ADDR, newVal * UV_REG_SHIFT + vrValue + WHITEBALANCE_OFFSET);

    if (err != DC1394_SUCCESS)
        cerr << "Could not set white balance register" << endl;
}

void Camera::setVR(int newVal, int uvValue)
{
    dc1394error_t	err;

    // Since UV and VR live in the same register, we need to take care of both
    err = dc1394_set_register(dc1394camera_, WHITEBALANCE_ADDR, newVal + UV_REG_SHIFT * uvValue + WHITEBALANCE_OFFSET);

    if (err != DC1394_SUCCESS)
        cerr << "Could not set white balance register" << endl;
}

uint32_t Camera::getWhiteBalance() const
{
    uint32_t wb;
    dc1394_get_register(dc1394camera_, WHITEBALANCE_ADDR, &wb);
    return wb;
}

void Camera::setWhiteBalance(uint32_t wb)
{
    dc1394error_t	err;

    // Since UV and VR live in the same register, we need to take care of both
    err = dc1394_set_register(dc1394camera_, WHITEBALANCE_ADDR, wb);

    if (err != DC1394_SUCCESS)
        cerr << "Could not set white balance register" << endl;
}

void Camera::setExternTrigger(bool on)
{
    dc1394error_t	err;
    err = dc1394_external_trigger_set_power(dc1394camera_, on ? DC1394_ON : DC1394_OFF);

    if (err != DC1394_SUCCESS)
        cerr << "Couldn't set external trigger." << endl;
}
