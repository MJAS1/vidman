#include <sched.h>
#include <time.h>
#include <cstring>
#include <QImage>
#include <QCoreApplication>
#include "settings.h"
#include "cameraworker.h"
#include "camera.h"
#include "cycdatabuffer.h"
#include "common.h"
#include "config.h"

CameraWorker::CameraWorker(CycDataBuffer* cycBuf, Camera &cam) : cycBuf_(cycBuf),
    cam_(cam), trigCode_(0), shouldStop_(false)
{
    Settings settings;

    if(settings.flip)
        preEvents_.insertSorted(EventPtr(new FlipEvent(0, 0)));

    if(settings.turnAround)
        preEvents_.insertSorted(EventPtr(new RotateEvent(0, 180, 0)));

    if(settings.fixPoint) {
        //fixPoint.png is stored in qt resource file, so it needs to be loaded to QImage first
        QImage fixImg(":/img/fixPoint.png");
        cv::Mat fixMat = cv::Mat(fixImg.height(), fixImg.width(), CV_8UC4, fixImg.bits(), fixImg.bytesPerLine()).clone();
        cv::cvtColor(fixMat, fixMat, CV_RGBA2BGRA);

        if(!fixMat.empty())
            preEvents_.insertSorted(EventPtr(new ImageEvent(0,cv::Point2i((VIDEO_WIDTH-fixMat.cols)/2, (VIDEO_HEIGHT-fixMat.rows)/2), fixMat, 0)));
        else
            std::cerr << "Couldn't load fixPoint.png" << std::endl;
    }

    cam_.setFPS(settings.fps);
}

/* start() and stop() are called by the main thread. However, the loop is
 * supposed to be working in a different thread. invokeMethod() makes sure that
 * the call for startLoop() and  stopLoop is placed in the thread's event queue.
 */
void CameraWorker::start()
{
    QMetaObject::invokeMethod(this, "startLoop", Qt::QueuedConnection);
}

void CameraWorker::stop()
{
    QMetaObject::invokeMethod(this, "stopLoop", Qt::BlockingQueuedConnection);
}

void CameraWorker::startLoop()
{
    // Set priority
    struct sched_param sch_param;
    sch_param.sched_priority = CAM_THREAD_PRIORITY;
    if (sched_setscheduler(0, SCHED_FIFO, &sch_param))
        std::cerr << "Cannot set camera thread priority. Continuing "
                     "nevertheless, but don't blame me if you experience"
                     " any strange problems." << std::endl;

    while(!shouldStop_) {
        /*--------------------------------------------------------------------
         *  have the camera start sending us data
         *--------------------------------------------------------------------*/

        cam_ >> frame_;
        if (frame_.empty()) {
            std::cerr << "Error dequeuing a frame" << std::endl;
            abort();
        }

        struct timespec	timestamp;
        clock_gettime(CLOCK_REALTIME, &timestamp);

        uint64_t msec;
        msec = timestamp.tv_nsec / 1000000;
        msec += timestamp.tv_sec * 1000;

        preEvents_.applyEvents(frame_);

        mutex_.lock();
        events_.applyEvents(frame_);
        mutex_.unlock();

        //Some video event may have emitted a signal, so process events before
        //continuing.
        QCoreApplication::processEvents();

        cv::cvtColor(frame_, frame_, CV_BGR2RGB);

        ChunkAttrib chunkAttrib;
        strncpy(chunkAttrib.log, log_.toStdString().c_str(), MAXLOG);
        chunkAttrib.log[MAXLOG-1] = '\0';
        log_.clear();

        chunkAttrib.trigCode = trigCode_;
        trigCode_ = 0;

        chunkAttrib.chunkSize = VIDEO_HEIGHT * VIDEO_WIDTH * 3;
        chunkAttrib.timestamp = msec;

        cycBuf_->insertChunk(frame_.data, chunkAttrib);
    }
}

void CameraWorker::stopLoop()
{
    shouldStop_ = true;
}

void CameraWorker::clearEvents()
{
    mutex_.lock();
    events_.clear();
    mutex_.unlock();
}

void CameraWorker::pause()
{
    mutex_.lock();
    events_.pauseEvents();
    mutex_.unlock();
}

void CameraWorker::unpause()
{
    mutex_.lock();
    events_.unpauseEvents();
    mutex_.unlock();
}

void CameraWorker::handleEvent(EventPtr ev)
{
    connect(ev.get(), SIGNAL(triggered(int, const QString&)), this,
            SLOT(onEventTriggered(int, const QString&)));

    mutex_.lock();
    events_.insertSorted(std::move(ev));
    mutex_.unlock();
}

void CameraWorker::onEventTriggered(int trigCode, const QString& log)
{
    trigCode_ = trigCode;
    log_ = log;
}

void CameraWorker::motionDialogToggled(bool on)
{
    if(on) {
        EventPtr movement(new MotionDetectorEvent(0, 0, 0, 0));
        connect(movement.get(), SIGNAL(pixmapReady(const QPixmap&)), this,
                SIGNAL(motionPixmapReady(const QPixmap&)));
        connect(this, SIGNAL(motionDialogColorChanged(bool)), movement.get(),
                SLOT(changeMovementFrameColor(bool)));
        preEvents_.insertSorted(std::move(movement));
    }
    else {
        preEvents_.deleteType(Event::EVENT_DETECT_MOTION);
    }
}
