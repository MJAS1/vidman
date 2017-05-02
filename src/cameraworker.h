#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include "eventcontainer.h"
#include "event.h"

class CycDataBuffer;
class Camera;

/*!
 * This class acquires, timestamps and processes frames for a single Camera. It
 * should be moved to a separate QThread with QObject::moveToThread() before
 * calling start()
 */

class CameraWorker : public QObject
{
    Q_OBJECT
public:
    explicit CameraWorker(CycDataBuffer* cycBuf, Camera& cam);

    void handleEvent(EventPtr ev);
    void clearEvents();

    void pause();
    void unpause();

signals:
    void motionPixmapReady(const QPixmap&);

public slots:
    void motionDialogToggled(bool);
    void captureFrame();

private slots:
    void onEventTriggered(int trigCode, const QString&);

private:
    CycDataBuffer*  cycBuf_;
    Camera&         cam_;
    EventContainer  events_;
    EventContainer  defaultEvents_;

    QMutex          mutex_;
    QString         log_;
    cv::Mat         frame_;

    int             trigCode_;
    int             defaultTrig1_;
    int             defaultTrig2_;
    bool            shouldStop_;
};

#endif // CAMERAWORKER_H
