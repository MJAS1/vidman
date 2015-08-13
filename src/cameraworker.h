#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include "eventcontainer.h"
#include "event.h"

class CycDataBuffer;
class Camera;

/*! This class acquires, timestamps and processes frames for a single Camera. It
 * should be moved to a separate QThread with QObject::moveToThread() before
 * calling start()
 */

class CameraWorker : public QObject
{
    Q_OBJECT
public:
    explicit CameraWorker(CycDataBuffer* cycBuf, Camera &cam);

    void clearEvents();
    void pause();
    void unpause();

    void handleEvent(EventPtr ev);

public slots:
    void start();
    void stop();

signals:
    void motionPixmapReady(const QPixmap&);
    void motionDialogColorChanged(bool);

private slots:
    void grabFrame();
    void onEventTriggered(int trigCode, const QString&);


private:
    CycDataBuffer*  cycBuf_;

    QTimer          *timer_;
    Camera&         cam_;

    cv::Mat         frame_;

    EventContainer  events_, preEvents_;

    QString         log_;

    int             trigCode_;
    bool            shouldStop_;

    QMutex          mutex_;
};

#endif // CAMERAWORKER_H
