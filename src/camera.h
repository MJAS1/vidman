#ifndef CAMERA_H
#define CAMERA_H

#include <QMutex>
#include <opencv2/opencv.hpp>
#include <dc1394/dc1394.h>

class Camera
{
public:
    Camera();
    ~Camera();

    void        setFPS(int fps);
    bool        empty() const;

    uint32_t    getShutter() const;
    uint32_t    getGain() const;
    void        setShutter(int newVal);
    void        setGain(int newVal);
    void        setUV(int newVal, int vrValue);
    void        setVR(int newVal, int uvValue);

    //Allows the camera to be controlled by an external trigger source
    void        setExternTrigger(bool on);
    void        operator>>(cv::Mat& frame);

private:
    Camera(const Camera&);
    Camera& operator=(const Camera&);

    cv::VideoCapture        capCam_;
    dc1394camera_t*         dc1394camera_;
    dc1394_t*               dc1394Context_;
    QMutex                  mutex_;

    bool                    initialized_;
};

#endif // CAMERA_H
