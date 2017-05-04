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
 * should be moved to the same QThread with GLWorker to ensure they are
 * correctly synced together.
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
    //Emit a QPixmap showing motion, so it can be drawn e.g. to a MotionDialog.
    void motionPixmapReady(const QPixmap&);

public slots:
    /* If the motionDialog is shown, a motion detector event has to be added to
     * defaultEvents_. When the dialog is hidden the event is again deleted. */
    void motionDialogToggled(bool);

    //Grab a new frame, process it and insert it to cycBuf_.
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
};

#endif // CAMERAWORKER_H
