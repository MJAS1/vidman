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
    QDialog(parent), ui(new Ui::VideoDialog), window(window), keepLog(false), trigPortFd(-1), trigPort(PORT_NULL)
{
    ui->setupUi(this);

    int swapInterval = 0;
    if(settings.vsync)
        swapInterval = 1;

    /*Setup GLVideoWidget for drawing video frames. SwapInterval is used to sync
      trigger signals with screen refresh rate. */
    QGLFormat format;
    format.setSwapInterval(swapInterval);
    glVideoWidget = new GLVideoWidget(format, this);
    ui->verticalLayout->addWidget(glVideoWidget);
    ui->verticalLayout->setStretchFactor(glVideoWidget, 10);

    Settings settings;

    if(initVideo())
    {
        // Setup gain/shutter sliders
        uint32_t shutter;
        dc1394_get_register(camera, SHUTTER_ADDR, &shutter);
        ui->shutterSlider->setMinimum(SHUTTER_MIN_VAL);
        ui->shutterSlider->setMaximum(SHUTTER_MAX_VAL);
        ui->shutterSlider->setValue(shutter - SHUTTER_OFFSET);

        uint32_t gain;
        dc1394_get_register(camera, GAIN_ADDR, &gain);
        ui->gainSlider->setMinimum(GAIN_MIN_VAL);
        ui->gainSlider->setMaximum(GAIN_MAX_VAL);
        ui->gainSlider->setValue(gain - GAIN_OFFSET);

        ui->uvSlider->setMinimum(UV_MIN_VAL);
        ui->uvSlider->setMaximum(UV_MAX_VAL);

        ui->vrSlider->setMinimum(VR_MIN_VAL);
        ui->vrSlider->setMaximum(VR_MAX_VAL);

        // Set up video recording
        cycVideoBufRaw = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ);
        cycVideoBufJpeg = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ);
        cameraThread = new CameraThread(capCam, cycVideoBufRaw);
        videoFileWriter = new VideoFileWriter(cycVideoBufJpeg, settings.storagePath);
        videoCompressorThread = new VideoCompressorThread(cycVideoBufRaw, cycVideoBufJpeg, settings.jpgQuality);

        connect(cycVideoBufRaw, SIGNAL(chunkReady(unsigned char*, int)), glVideoWidget, SLOT(onDrawFrame(unsigned char*, int)));

        // Start video running
        videoFileWriter->start();
        videoCompressorThread->start();
        cameraThread->start();

        eventTmr = new QTimer(this);
        eventTmr->setSingleShot(true);
        connect(eventTmr, SIGNAL(timeout()), this, SLOT(getNextEvent()));

        events = new EventContainer<Event*>();
    }
    else
    {
        videoAvailable = false;
        window->toggleStart(false);
        ui->shutterSlider->setEnabled(false);
        ui->gainSlider->setEnabled(false);
        ui->uvSlider->setEnabled(false);
        ui->vrSlider->setEnabled(false);
        QMessageBox msgBox;
        msgBox.setText("Couldn't initialize video.");
        msgBox.exec();
    }


}

VideoDialog::~VideoDialog()
{
    if(videoAvailable)
    {
        stopThreads();
        free(dc1394Context);
        dc1394_camera_free(camera);
        delete events;
        delete capCam;
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
    keepLog = arg;
}

void VideoDialog::toggleRecord(bool arg)
{
    cycVideoBufJpeg->setIsRec(arg);
}

void VideoDialog::getNextEvent()
{
    Event *event = events->pop_front();

    switch(event->getType())
    {

        case EVENT_DETECT_MOTION:
            cameraThread->detectMotion(event);
            break;

        case EVENT_REMOVE:
            cameraThread->removeEvent(static_cast<RemoveEvent*>(event));
            break;

        default:
            cameraThread->addVideoEvent(static_cast<VideoEvent*>(event));
            break;
    }

    if(!events->empty())
    {
        Event *nextEvent = (*events)[0];
        eventDuration = (nextEvent->getStart()+event->getDuration()+event->getDelay());
        eventTmr->start(eventDuration);
        time=elapsedTimer.nsecsElapsed()/1000000;
    }
}

bool VideoDialog::start(const QString& eventStr)
{
    if(keepLog)
    {
        logFile.setFileName(QDateTime::currentDateTime().toString(QString("yyyy-MM-dd--hh:mm:ss.log")));
        logFile.open(QFile::WriteOnly | QFile::Truncate);
        if(!logFile.isOpen())
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
        if(!events->empty())
        {
            eventTmr->start((*events)[0]->getStart());
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
    eventTmr->stop();
    events->clear();
    logFile.close();
}

void VideoDialog::pause()
{
    eventTmr->stop();
    cameraThread->pause();
    time = elapsedTimer.nsecsElapsed()/1000000-time;
}

void VideoDialog::unpause()
{
    cameraThread->unpause();
    if(!events->empty())
    {
        eventDuration = eventDuration - time;
        eventTmr->start(eventDuration);
    }
    time = elapsedTimer.nsecsElapsed()/1000000;
}

void VideoDialog::onShutterChanged(int newVal)
{
    dc1394error_t	err;

    err = dc1394_set_register(camera, SHUTTER_ADDR, newVal + SHUTTER_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Could not set shutter register" << std::endl;
        //abort();
    }
}

void VideoDialog::onGainChanged(int newVal)
{
    dc1394error_t	err;

    err = dc1394_set_register(camera, GAIN_ADDR, newVal + GAIN_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Could not set gain register" << std::endl;
        //abort();
    }
}


void VideoDialog::onUVChanged(int newVal)
{
    dc1394error_t	err;

    // Since UV and VR live in the same register, we need to take care of both
    err = dc1394_set_register(camera, WHITEBALANCE_ADDR, newVal * UV_REG_SHIFT + ui->vrSlider->value() + WHITEBALANCE_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Could not set white balance register" << std::endl;
        //abort();
    }
}


void VideoDialog::onVRChanged(int newVal)
{
    dc1394error_t	err;

    // Since UV and VR live in the same register, we need to take care of both
    err = dc1394_set_register(camera, WHITEBALANCE_ADDR, newVal + UV_REG_SHIFT * ui->uvSlider->value() + WHITEBALANCE_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Could not set white balance register" << std::endl;
        //abort();
    }
}

void VideoDialog::onWidthChanged(int newVal)
{
    glVideoWidget->setVideoWidth(newVal);
}

bool VideoDialog::initVideo()
{
    capCam = new cv::VideoCapture;

    capCam->open(300);
    if(!capCam->isOpened())
    {
        std::cerr << "No cameras found" << std::endl;
        delete capCam;
        return false;
    }


    dc1394camera_list_t*	camList;
    dc1394error_t			err;

    dc1394Context = dc1394_new();
    if(!dc1394Context)
    {
        std::cerr << "Cannot initialize!" << std::endl;
        delete capCam;
        return false;
    }

    err = dc1394_camera_enumerate(dc1394Context, &camList);
    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Failed to enumerate cameras" << std::endl;
        free(dc1394Context);
        delete capCam;
        return false;
    }

    camera = NULL;

    if (camList->num == 0)
    {
        std::cerr << "No cameras found" << std::endl;
        free(dc1394Context);
        delete capCam;
        return false;
    }

    // use the first camera in the list
    camera = dc1394_camera_new(dc1394Context, camList->ids[0].guid);
    if (!camera)
    {
        std::cerr << "Failed to initialize camera with guid " << camList->ids[0].guid << std::endl;
        free(dc1394Context);
        delete capCam;
        return false;
    }
    std::cout << "Using camera with GUID " << camera->guid << std::endl;

    /*
    err = dc1394_video_set_operation_mode(camera, DC1394_OPERATION_MODE_1394B);
    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Could not set operation mode" << std::endl;
        free(dc1394Context);
        delete capCam;
        delete camera;
        return false;
    }

    err = dc1394_video_set_iso_speed(camera, DC1394_ISO_SPEED_MAX);
    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Failed to set ISO speed" << std::endl;
        free(dc1394Context);
        delete capCam;
        delete camera;
        return false;
    }
    */
    dc1394_camera_free_list(camList);

    videoAvailable = true;
    return true;
}

void VideoDialog::closeEvent(QCloseEvent *)
{
    window->toggleVideoDialogChecked(false);
}

void VideoDialog::writeToLogFile(QString log)
{
    if(keepLog)
    {
        qint64 elapsedTime = window->getRunningTime();

        QTextStream logStream(&logFile);
        logStream << "[" << elapsedTime/1000000000 << "s "
                  << (elapsedTime%1000000000)/1000000 << "ms]"
                  << log << "\n";
    }
}

bool VideoDialog::setTrigPort(PortType port)
{

    if(port == PORT_PRINTER)
    {
        if (ioperm(settings.printerPortAddr, 1, 1))
        {
            QMessageBox msgBox;
            msgBox.setText("Cannot get the port. May be you should run this program as root\n");
            msgBox.exec();
            trigPort = PORT_NULL;
            return false;
        }
    }
    else if(port == PORT_USB)
    {
        if((trigPortFd = ::open("/dev/ttyUSB0", O_RDWR)) < 1)
        {
            QMessageBox msgBox;
            msgBox.setText("Cannot open USB port.");
            msgBox.exec();
            trigPort = PORT_NULL;
            trigPortFd = -1;
            return false;
        }
    }

    trigPort = port;

    return true;
}

void VideoDialog::setFPS(int fps)
{
    ui->FPSLabel->setText(QString("FPS: %1").arg(fps));
}

void VideoDialog::sendTrigSignal(int trigCode) const
{
    if(trigCode)

        if(trigPort == PORT_USB)
        {
            if(ioctl(trigPortFd, TIOCMSET, &trigCode) == -1)
            {
                fprintf(stderr, "Cannot open port: %s\n", strerror(errno));
            }
        }
        else if(trigPort == PORT_PRINTER)
        {
            outb(trigCode, settings.printerPortAddr);
        }
}

void VideoDialog::updateBackground()
{
    cameraThread->updateBackground();
}
