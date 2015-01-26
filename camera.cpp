#include <unistd.h>
#include <QMutexLocker>
#include "camera.h"

Camera::Camera() : initialized_(false)
{
    //Initialize camera
    capCam_.open(300);
    if(!capCam_.isOpened())
    {
        std::cerr << "No cameras found" << std::endl;
        return;
    }

    dc1394camera_list_t*	camList;
    dc1394error_t			err;

    dc1394Context_ = dc1394_new();
    if(!dc1394Context_)
    {
        std::cerr << "Cannot initialize!" << std::endl;
        return;
    }

    err = dc1394_camera_enumerate(dc1394Context_, &camList);
    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Failed to enumerate cameras" << std::endl;
        free(dc1394Context_);
        return;
    }

    dc1394camera_ = NULL;

    if (camList->num == 0)
    {
        std::cerr << "No cameras found" << std::endl;
        free(dc1394Context_);
        return;
    }

    // use the first camera in the list
    dc1394camera_ = dc1394_camera_new(dc1394Context_, camList->ids[0].guid);
    if (!dc1394camera_)
    {
        std::cerr << "Failed to initialize camera with guid " << camList->ids[0].guid << std::endl;
        free(dc1394Context_);
        return;
    }
    std::cout << "Using camera with GUID " << dc1394camera_->guid << std::endl;

    dc1394_camera_free_list(camList);

    initialized_ = true;
}

Camera::~Camera()
{
    if(initialized_)
    {
        free(dc1394Context_);
        free(dc1394camera_);
    }
}

void Camera::setFPS(int fps)
{
    mutex_.lock();
    if(!capCam_.set(CV_CAP_PROP_FPS, fps))
    {
        std::cerr << "Could not set framerate" << std::endl;
        abort();
    }
    mutex_.unlock();
}

void Camera::operator >>(cv::Mat& frame)
{
    mutex_.lock();
    capCam_ >> frame;
    mutex_.unlock();
}

bool Camera::isInitialized() const
{
    return initialized_;
}

uint32_t Camera::getShutter() const
{
    mutex_.lock();
    uint32_t shutter;
    dc1394_get_register(dc1394camera_, SHUTTER_ADDR, &shutter);
    mutex_.unlock();
    return shutter;
}
uint32_t Camera::getGain() const
{
    mutex_.lock();
    uint32_t gain;
    dc1394_get_register(dc1394camera_, GAIN_ADDR, &gain);
    mutex_.unlock();
    return gain;
}

void Camera::setShutter(int newVal)
{
    mutex_.lock();
    dc1394error_t	err;

    err = dc1394_set_register(dc1394camera_, SHUTTER_ADDR, newVal + SHUTTER_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Could not set shutter register" << std::endl;
    }
    mutex_.unlock();
}

void Camera::setGain(int newVal)
{
    mutex_.lock();
    dc1394error_t	err;

    err = dc1394_set_register(dc1394camera_, GAIN_ADDR, newVal + GAIN_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Could not set gain register" << std::endl;
    }
    mutex_.unlock();
}

void Camera::setUV(int newVal, int vrValue)
{
    mutex_.lock();
    dc1394error_t	err;

    // Since UV and VR live in the same register, we need to take care of both
    err = dc1394_set_register(dc1394camera_, WHITEBALANCE_ADDR, newVal * UV_REG_SHIFT + vrValue + WHITEBALANCE_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Could not set white balance register" << std::endl;
        //abort();
    }
    mutex_.unlock();
}

void Camera::setVR(int newVal, int uvValue)
{
    mutex_.lock();
    dc1394error_t	err;

    // Since UV and VR live in the same register, we need to take care of both
    err = dc1394_set_register(dc1394camera_, WHITEBALANCE_ADDR, newVal + UV_REG_SHIFT * uvValue + WHITEBALANCE_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Could not set white balance register" << std::endl;
        //abort();
    }
    mutex_.unlock();
}

void Camera::setExternTrigger(bool on)
{
    QMutexLocker locker(&mutex_);
    dc1394error_t	err;

    err = dc1394_external_trigger_set_power(dc1394camera_, on ? DC1394_ON : DC1394_OFF);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Couldn't set external trigger." << std::endl;
        //abort();
    }
}
