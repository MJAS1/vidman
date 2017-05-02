#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>
#include <dc1394/dc1394.h>

/*!
 * This class is used to control the videocamera. It uses dc1394 library to
 * access the camera's register and OpenCV to acquire frames.
 */

class Camera
{
public:
    Camera();
    ~Camera();

    bool        empty() const;

    uint32_t    getShutter() const;
    uint32_t    getGain() const;
    uint32_t    getWhiteBalance() const;
    void        setShutter(int newVal);
    void        setGain(int newVal);
    void        setUV(int newVal, int vrValue);
    void        setVR(int newVal, int uvValue);
    void        setWhiteBalance(uint32_t wb);
    void        setFPS(int fps);

    //Allows the camera to be controlled by an external trigger source
    void        setExternTrigger(bool on);
    void        operator>>(cv::Mat& frame);

private:
    Camera(const Camera&);
    Camera& operator=(const Camera&);

    cv::VideoCapture        capCam_;
    dc1394camera_t*         dc1394camera_;
    dc1394_t*               dc1394Context_;

    bool                    empty_;
};

#endif // CAMERA_H
