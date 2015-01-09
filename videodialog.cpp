#include <QTimer>
#include <QMessageBox>
#include <QTextStream>
#include <QDateTime>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/io.h>
#include <fcntl.h>
#include <termios.h>
#include <cerrno>
#include <stdio.h>
#include <QGLFormat>
#include "glvideowidget.h"
#include "mainwindow.h"
#include "videodialog.h"
#include "ui_videodialog.h"
#include "settings.h"
#include "config.h"
#include "videoevent.h"

VideoDialog::VideoDialog(MainWindow *window, QWidget *parent) :
    QDialog(parent), ui(new Ui::VideoDialog), window(window), logFile(window->getTimer())
{
    ui->setupUi(this);

    int swapInterval = 0;
    if(settings.vsync)
        swapInterval = 1;

    /*Setup GLVideoWidget for drawing video frames. SwapInterval is used to sync
      trigger signals with screen refresh rate. */
    QGLFormat format;
    format.setSwapInterval(swapInterval);
    glVideoWidget = new GLVideoWidget(format, trigPort, logFile, this);
    ui->verticalLayout->addWidget(glVideoWidget);
    ui->verticalLayout->setStretchFactor(glVideoWidget, 10);

    if(camera.init())
    {
        // Setup gain/shutter sliders
        ui->shutterSlider->setMinimum(SHUTTER_MIN_VAL);
        ui->shutterSlider->setMaximum(SHUTTER_MAX_VAL);
        ui->shutterSlider->setValue(camera.getShutter() - SHUTTER_OFFSET);


        ui->gainSlider->setMinimum(GAIN_MIN_VAL);
        ui->gainSlider->setMaximum(GAIN_MAX_VAL);
        ui->gainSlider->setValue(camera.getGain() - GAIN_OFFSET);

        ui->uvSlider->setMinimum(UV_MIN_VAL);
        ui->uvSlider->setMaximum(UV_MAX_VAL);

        ui->vrSlider->setMinimum(VR_MIN_VAL);
        ui->vrSlider->setMaximum(VR_MAX_VAL);

        // Set up video recording
        cycVideoBufRaw = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ);
        cycVideoBufJpeg = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ);
        cameraThread = new CameraThread(camera, cycVideoBufRaw);
        videoFileWriter = new VideoFileWriter(cycVideoBufJpeg, settings.storagePath);
        videoCompressorThread = new VideoCompressorThread(cycVideoBufRaw, cycVideoBufJpeg, settings.jpgQuality);

        connect(cycVideoBufRaw, SIGNAL(chunkReady(unsigned char*, int)), glVideoWidget, SLOT(onDrawFrame(unsigned char*, int)));

        // Start video running
        videoFileWriter->start();
        videoCompressorThread->start();
        cameraThread->start();

        eventTmr.setSingleShot(true);
        connect(&eventTmr, SIGNAL(timeout()), this, SLOT(getNextEvent()));

    }
    else
    {
        videoAvailable = false;
        ui->shutterSlider->setEnabled(false);
        ui->gainSlider->setEnabled(false);
        ui->uvSlider->setEnabled(false);
        ui->vrSlider->setEnabled(false);
        QMessageBox msgBox;
        msgBox.setText("Couldn't initialize video.");
        msgBox.exec();
        window->toggleStart(false);
        window->toggleRec(false);
    }


}

VideoDialog::~VideoDialog()
{
    if(camera.isInitialized())
    {
        stopThreads();
        delete cycVideoBufRaw;
        delete cycVideoBufJpeg;
        delete cameraThread;
        delete videoFileWriter;
        delete videoCompressorThread;
    }

    delete ui;
}

void VideoDialog::stopThreads()
{
    // The piece of code stopping the threads should execute fast enough,
    // otherwise cycVideoBufRaw or cycVideoBufJpeg buffer might overflow. The
    // order of stopping the threads is important.
    videoFileWriter->stop();
    videoCompressorThread->stop();
    cameraThread->stop();
}

void VideoDialog::setKeepLog(bool arg)
{
    logFile.setActive(arg);
}

void VideoDialog::toggleRecord(bool arg)
{
    cycVideoBufJpeg->setIsRec(arg);
}

void VideoDialog::getNextEvent()
{
    Event *event = events.pop_front();

    switch(event->getType())
    {

        case Event::EVENT_DETECT_MOTION:
            cameraThread->detectMotion(event);
            break;

        case Event::EVENT_REMOVE:
            cameraThread->removeEvent(static_cast<RemoveEvent*>(event));
            break;

        default:
            cameraThread->addVideoEvent(static_cast<VideoEvent*>(event));
            break;
    }

    if(!events.empty())
    {
        Event *nextEvent = events[0];
        eventDuration = (nextEvent->getStart()+event->getDuration()+event->getDelay());
        eventTmr.start(eventDuration);
        time=elapsedTimer.nsecsElapsed()/1000000;
    }
}

bool VideoDialog::start(const QString& eventStr)
{
    if(logFile.isActive())
    {
        if(!logFile.open())
        {
            QMessageBox msgBox;
            msgBox.setText(QString("Error creating log file."));
            msgBox.exec();
            return false;
        }
    }

    QStringList strList = eventStr.split("\n");
    strList.append("");

    EventReader eventReader;
    if(eventReader.loadEvents(strList, events))
    {
        if(!events.empty())
        {
            eventTmr.start(events[0]->getStart());
            time=0;
        }
        elapsedTimer.restart();

        return true;
    }

    else
        return false;
}

void VideoDialog::stop()
{
    cameraThread->clearEvents();
    eventTmr.stop();
    events.clear();
    logFile.close();
}

void VideoDialog::pause()
{
    eventTmr.stop();
    cameraThread->pause();
    time = elapsedTimer.nsecsElapsed()/1000000-time;
}

void VideoDialog::unpause()
{
    cameraThread->unpause();
    if(!events.empty())
    {
        eventDuration = eventDuration - time;
        eventTmr.start(eventDuration);
    }
    time = elapsedTimer.nsecsElapsed()/1000000;
}
void VideoDialog::onShutterChanged(int newVal)
{
        camera.setShutter(newVal);
}

void VideoDialog::onGainChanged(int newVal)
{
        camera.setGain(newVal);
}


void VideoDialog::onUVChanged(int newVal)
{
        camera.setUV(newVal, ui->vrSlider->value());
}

void VideoDialog::onVRChanged(int newVal)
{
        camera.setVR(newVal, ui->uvSlider->value());
}

void VideoDialog::onWidthChanged(int newVal)
{
    glVideoWidget->setVideoWidth(newVal);
}

void VideoDialog::closeEvent(QCloseEvent *)
{
    window->toggleVideoDialogChecked(false);
}

void VideoDialog::setFPS(int fps)
{
    ui->FPSLabel->setText(QString("FPS: %1").arg(fps));
}


void VideoDialog::updateBackground()
{
    cameraThread->updateBackground();
}

bool VideoDialog::setOutputDevice(OutputDevice::PortType portType)
{
    if(portType)
    {
        if(!trigPort.open(portType))
            return false;
    }
    else
        trigPort.close();

    return true;
}

void VideoDialog::onExternTrig(bool on)
{
    camera.setExternTrigger(on);
}
