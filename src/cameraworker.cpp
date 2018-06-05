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

CameraWorker::CameraWorker(CycDataBuffer* cycBuf, Camera &cam):
    cycBuf_(cycBuf), cam_(cam), trigCode_(0)
{
    qRegisterMetaType<uint8_t>("uint8_t");

    events_ = new EventContainer(this);
    defaultEvents_ = new EventContainer(this);

    Settings settings;  
    defaultTrig1_ = settings.defaultTrig1;
    defaultTrig2_ = settings.defaultTrig2;

    //Setup defaultEvents for processing each frame captured by the camera.
    if(settings.flip) {
        int code = settings.flipCode;
        defaultEvents_->insertSorted(EventPtr(new FlipEvent(0, code)));
    }
    if(settings.fixPoint) {
        /*
         * fixPoint.png is stored in the qt resource file, so it needs to be
         * loaded to a QImage before converting to cv::Mat.
         */
        QImage fixImg(":/img/fixPoint.png");
        cv::Mat fixMat = cv::Mat(fixImg.height(), fixImg.width(), CV_8UC4,
                                 fixImg.bits(), fixImg.bytesPerLine()).clone();
        cv::cvtColor(fixMat, fixMat, CV_RGBA2BGRA);

        if(!fixMat.empty()) {
            cv::Point2i pos((VIDEO_WIDTH-fixMat.cols)/2,
                            (VIDEO_HEIGHT-fixMat.rows)/2);
            EventPtr imgEvent(new ImageEvent(0, pos, fixMat, 0));
            defaultEvents_->insertSorted(std::move(imgEvent));
        }
        else
            std::cerr << "Couldn't load fixPoint.png" << std::endl;
    }

    if(!cam_.empty())
        cam_.setFPS(settings.fps);
}

void CameraWorker::captureFrame()
{
    /*
     * Set default trigger code for each frame. Toggle between defaultTrig1_ and
     * defaultTrig2.
     */
    trigCode_ = (trigCode_ == defaultTrig1_ ? defaultTrig2_: defaultTrig1_);
    log_.clear();

    cam_ >> frame_;
    if (frame_.empty()) {
        std::cerr << "Error dequeuing a frame." << std::endl;
        return;
    }

    struct timespec	timestamp;
    clock_gettime(CLOCK_REALTIME, &timestamp);

    uint64_t msec;
    msec = timestamp.tv_nsec / 1000000;
    msec += timestamp.tv_sec * 1000;

    defaultEvents_->applyEvents(frame_);

    mutex_.lock();
    events_->applyEvents(frame_);
    mutex_.unlock();

    // Process signals emitted by events_->applyEvents().
    QCoreApplication::processEvents();
    cv::cvtColor(frame_, frame_, CV_BGR2RGB);

    ChunkAttrib chunkAttrib;
    strncpy(chunkAttrib.log, log_.toStdString().c_str(), MAXLOG);
    chunkAttrib.log[MAXLOG-1] = '\0';
    chunkAttrib.trigCode = trigCode_;
    chunkAttrib.chunkSize = VIDEO_HEIGHT * VIDEO_WIDTH * 3;
    chunkAttrib.timestamp = msec;

    cycBuf_->insertChunk(frame_.data, chunkAttrib);
}

void CameraWorker::clearEvents()
{
    mutex_.lock();
    events_->clear();
    mutex_.unlock();
}

void CameraWorker::pause()
{
    mutex_.lock();
    events_->pauseEvents();
    mutex_.unlock();
}

void CameraWorker::unpause()
{
    mutex_.lock();
    events_->unpauseEvents();
    mutex_.unlock();
}

void CameraWorker::addEvent(EventPtr ev)
{
    connect(ev.get(), SIGNAL(triggered(uint8_t, const QString&)), this,
            SLOT(onEventTriggered(uint8_t, const QString&)));

    mutex_.lock();
    events_->insertSorted(std::move(ev));
    mutex_.unlock();
}

void CameraWorker::onEventTriggered(uint8_t trigCode, const QString& log)
{
    trigCode_ = trigCode;
    log_ = log;
}

void CameraWorker::motionDialogToggled(bool on)
{
    if(on) {
        EventPtr movement(new MotionDetectorEvent());
        connect(movement.get(), SIGNAL(pixmapReady(const QPixmap&)), this,
                SIGNAL(motionPixmapReady(const QPixmap&)));
        defaultEvents_->insertSorted(std::move(movement));
    }
    else {
        defaultEvents_->deleteType(Event::EVENT_DETECT_MOTION);
    }
}
