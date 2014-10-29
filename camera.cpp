#include "camera.h"

Camera::Camera() : initialized(false)
{
}

Camera::~Camera()
{
    if(initialized)
    {
        free(dc1394Context);
        free(dc1394camera);
    }
}

bool Camera::init()
{
    capCam.open(300);
    if(!capCam.isOpened())
    {
        std::cerr << "No cameras found" << std::endl;
        return false;
    }

    dc1394camera_list_t*	camList;
    dc1394error_t			err;

    dc1394Context = dc1394_new();
    if(!dc1394Context)
    {
        std::cerr << "Cannot initialize!" << std::endl;
        return false;
    }

    err = dc1394_camera_enumerate(dc1394Context, &camList);
    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Failed to enumerate cameras" << std::endl;
        free(dc1394Context);
        return false;
    }

    dc1394camera = NULL;

    if (camList->num == 0)
    {
        std::cerr << "No cameras found" << std::endl;
        free(dc1394Context);
        return false;
    }

    // use the first camera in the list
    dc1394camera = dc1394_camera_new(dc1394Context, camList->ids[0].guid);
    if (!dc1394camera)
    {
        std::cerr << "Failed to initialize camera with guid " << camList->ids[0].guid << std::endl;
        free(dc1394Context);
        return false;
    }
    std::cout << "Using camera with GUID " << dc1394camera->guid << std::endl;

    dc1394_camera_free_list(camList);

    return true;
}

void Camera::setFPS(int fps)
{
    if(!capCam.set(CV_CAP_PROP_FPS, fps))
    {
        std::cerr << "Could not set framerate" << std::endl;
        abort();
    }
}

void Camera::operator >>(cv::Mat& frame)
{
    capCam >> frame;
}

bool Camera::isInitialized() const
{
    return initialized;
}

uint32_t Camera::getShutter() const
{
    uint32_t shutter;
    dc1394_get_register(dc1394camera, SHUTTER_ADDR, &shutter);
    return shutter;
}
uint32_t Camera::getGain() const
{
    uint32_t gain;
    dc1394_get_register(dc1394camera, GAIN_ADDR, &gain);
    return gain;
}

void Camera::setShutter(int newVal)
{
    dc1394error_t	err;

    err = dc1394_set_register(dc1394camera, SHUTTER_ADDR, newVal + SHUTTER_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Could not set shutter register" << std::endl;
    }
}

void Camera::setGain(int newVal)
{
    dc1394error_t	err;

    err = dc1394_set_register(dc1394camera, GAIN_ADDR, newVal + GAIN_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Could not set gain register" << std::endl;
    }
}

void Camera::setUV(int newVal, int vrValue)
{

    dc1394error_t	err;

    // Since UV and VR live in the same register, we need to take care of both
    err = dc1394_set_register(dc1394camera, WHITEBALANCE_ADDR, newVal * UV_REG_SHIFT + vrValue + WHITEBALANCE_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Could not set white balance register" << std::endl;
        //abort();
    }
}

void Camera::setVR(int newVal, int uvValue)
{
    dc1394error_t	err;

    // Since UV and VR live in the same register, we need to take care of both
    err = dc1394_set_register(dc1394camera, WHITEBALANCE_ADDR, newVal + UV_REG_SHIFT * uvValue + WHITEBALANCE_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Could not set white balance register" << std::endl;
        //abort();
    }
}

void Camera::setExternTrigger(bool on)
{
    dc1394error_t	err;

    err = dc1394_external_trigger_set_power(dc1394camera, on ? DC1394_ON : DC1394_OFF);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Couldn't set external trigger." << std::endl;
        //abort();
    }
}
