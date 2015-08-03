#include <QMessageBox>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QGLFormat>
#include <memory>
#include "camerathread.h"
#include "glvideowidget.h"
#include "mainwindow.h"
#include "videodialog.h"
#include "ui_videodialog.h"
#include "config.h"
#include "videofilewriter.h"
#include "videocompressorthread.h"
#include "common.h"

using namespace std;

VideoDialog::VideoDialog(MainWindow *window) :
    QDialog(window), ui(new Ui::VideoDialog), window_(window)
{
    setWindowFlags(Qt::Window);
    ui->setupUi(this);

    /*Setup GLVideoWidget for drawing video frames. SwapInterval is used to sync
      trigger signals with screen refresh rate. */
    QGLFormat format;
    if(!format.hasOpenGL())
        cerr << "OpenGL not supported by window system. Cannot use vsync." << endl;

    format.setSwapInterval(settings_.vsync);
    glVideoWidget_ = new GLVideoWidget(format, this);
    ui->verticalLayout->addWidget(glVideoWidget_, 1);

    if(cam_.isInitialized()) {
        // Set up video recording
        cycVideoBufRaw_ = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ, this);
        cycVideoBufJpeg_ = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ, this);
        cameraThread_ = new CameraThread(cycVideoBufRaw_, cam_, this);
        videoFileWriter_ = new VideoFileWriter(cycVideoBufJpeg_, settings_.storagePath, this);
        videoCompressorThread_ = new VideoCompressorThread(cycVideoBufRaw_, cycVideoBufJpeg_, settings_.jpgQuality, this);

        connect(videoFileWriter_, SIGNAL(error(const QString&)), this, SLOT(fileWriterError(const QString&)));
        connect(cycVideoBufRaw_, SIGNAL(chunkReady(unsigned char*)), glVideoWidget_, SLOT(onDrawFrame(unsigned char*)));
        connect(cameraThread_, SIGNAL(motionDetectorPixmap(const QPixmap&)), window_, SLOT(updateMotionDetectorLabel(const QPixmap&)));
        connect(window_, SIGNAL(changeMotionDialogColors(bool)), cameraThread_, SLOT(changeMovementFrame(bool)));

        // Setup gain/shutter sliders
        ui->shutterSlider->setMinimum(SHUTTER_MIN_VAL);
        ui->shutterSlider->setMaximum(SHUTTER_MAX_VAL);
        ui->shutterSlider->setValue(cam_.getShutter() - SHUTTER_OFFSET);

        ui->gainSlider->setMinimum(GAIN_MIN_VAL);
        ui->gainSlider->setMaximum(GAIN_MAX_VAL);
        ui->gainSlider->setValue(cam_.getGain() - GAIN_OFFSET);

        ui->uvSlider->setMinimum(UV_MIN_VAL);
        ui->uvSlider->setMaximum(UV_MAX_VAL);

        ui->vrSlider->setMinimum(VR_MIN_VAL);
        ui->vrSlider->setMaximum(VR_MAX_VAL);

        // Start video running
        videoFileWriter_->start();
        videoCompressorThread_->start();
        cameraThread_->start();

        //Setup event handling
        eventTmr_.setSingleShot(true);
        connect(&eventTmr_, SIGNAL(timeout()), this, SLOT(getNextEvent()));
    }
    else {
        //If camera initializaton didn't work, disable all functionality
        ui->shutterSlider->setEnabled(false);
        ui->gainSlider->setEnabled(false);
        ui->uvSlider->setEnabled(false);
        ui->vrSlider->setEnabled(false);
        window_->setStatus(QString("Couldn't initialize video."));
        window_->toggleStart(false);
        window_->toggleRecEnabled(false);
        window_->toggleStop(false);
    }
}

VideoDialog::~VideoDialog()
{
    if(cam_.isInitialized())
        stopThreads();
    delete ui;
}

void VideoDialog::paintEvent(QPaintEvent *)
{
    //Paint the area under sliders and labels
    QPainter painter(this);
    painter.setPen(QColor(80, 80, 80));
    QLinearGradient gradient(QPoint(0, 0), QPoint(0, glVideoWidget_->pos().y()));
    gradient.setColorAt(0, QColor(150, 150, 150));
    gradient.setColorAt(0.5, QColor(140, 140, 140));
    gradient.setColorAt(1, QColor(80, 80, 80));
    painter.drawRect(glVideoWidget_->pos().x(), 0, glVideoWidget_->width()-1, glVideoWidget_->pos().y());
    painter.fillRect(glVideoWidget_->pos().x()+1, 1, glVideoWidget_->width()-2, glVideoWidget_->pos().y()-1, gradient);
}

void VideoDialog::stopThreads()
{
    // The piece of code stopping the threads should execute fast enough,
    // otherwise cycVideoBufRaw or cycVideoBufJpeg buffer might overflow. The
    // order of stopping the threads is important.
    videoFileWriter_->stop();
    videoCompressorThread_->stop();
    cameraThread_->stop();
}

void VideoDialog::toggleRecEnabledord(bool arg)
{
    cycVideoBufJpeg_->setIsRec(arg);
}

void VideoDialog::getNextEvent()
{
    //Get next event and pass it to cameraThread
    EventPtr event(events_->pop_front());
    int delay = event->getDelay();
    cameraThread_->handleEvent(std::move(event));

    //Calculate the start time of the next event
    if(!events_->empty()) {        
        time_ = elapsedTimer_.nsecsElapsed()/1000000;
        const EventPtr& nextEvent = (*events_)[0];
        eventDuration_ = (nextEvent->getStart()+delay);
        eventTmr_.start(eventDuration_);
    }
}

void VideoDialog::start(EventContainerPtr events)
{
    events_ = std::move(events);

    if(!events_->empty()) {
        eventTmr_.start((*events_)[0]->getStart());
        time_ = 0;
    }

    elapsedTimer_.restart();
}

void VideoDialog::stop()
{
    cameraThread_->clearEvents();
    eventTmr_.stop();
    if(events_)
        events_->clear();
}

void VideoDialog::pause()
{
    eventTmr_.stop();
    cameraThread_->pause();
    time_ = elapsedTimer_.nsecsElapsed()/1000000-time_;
}

void VideoDialog::unpause()
{
    cameraThread_->unpause();
    if(!events_->empty()) {
        eventDuration_ = eventDuration_ - time_;
        eventTmr_.start(eventDuration_);
    }
    time_ = elapsedTimer_.nsecsElapsed()/1000000;
}

void VideoDialog::onShutterChanged(int newVal)
{
    cam_.setShutter(newVal);
}

void VideoDialog::onGainChanged(int newVal)
{
    cam_.setGain(newVal);
}

void VideoDialog::onUVChanged(int newVal)
{
    cam_.setUV(newVal, ui->vrSlider->value());
}

void VideoDialog::onVRChanged(int newVal)
{
    cam_.setVR(newVal, ui->uvSlider->value());
}

void VideoDialog::onAspectRatioChanged(int newVal)
{
    glVideoWidget_->setVideoWidth(newVal);
}

void VideoDialog::closeEvent(QCloseEvent *)
{
    window_->toggleVideoDialogChecked(false);
}

void VideoDialog::updateFPS(int fps)
{
    ui->FPSLabel->setText(QString("FPS: %1").arg(fps));
}

void VideoDialog::setOutputDevice(OutputDevice::PortType portType)
{
    glVideoWidget_->setOutputDevice(portType);
}

void VideoDialog::onExternTrig(bool on)
{
    cam_.setExternTrigger(on);
}

MainWindow* VideoDialog::mainWindow() const
{
    return window_;
}

void VideoDialog::fileWriterError(const QString &str)
{
    cycVideoBufJpeg_->setIsRec(false);
    window_->toggleRecChecked(false);
    window_->setStatus(str);
}
