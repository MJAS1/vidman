#include <QTimer>
#include <QMessageBox>
#include <QTextStream>
#include <QDateTime>
#include <time.h>
#include "mainwindow.h"
#include "videodialog.h"
#include "ui_videodialog.h"
#include "settings.h"
#include "config.h"


VideoDialog::VideoDialog(MainWindow *window, QWidget *parent) :
    QDialog(parent), ui(new Ui::VideoDialog), window(window), isRec(false), keepLog(false)
{
    ui->setupUi(this);

    Settings settings;
    color = settings.color;

    // Allocate memory. Since we do not know whether the image is going to BW
    // or color, allocate for color since it requires more memory.
    imBuf =  new char[VIDEO_HEIGHT*VIDEO_WIDTH*3];
    if (!imBuf)
    {
        std::cerr << "Error allocating memory" << std::endl;
        exit(EXIT_FAILURE);
    }

    row_pointer[0] =  new unsigned char[VIDEO_WIDTH*3];
    if(!(row_pointer[0]))
    {
        std::cerr << "Error allocating memory" << std::endl;
        exit(EXIT_FAILURE);
    }

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

        ui->uvLabel->setEnabled(settings.color);
        ui->vrLabel->setEnabled(settings.color);
        ui->wbLabel->setEnabled(settings.color);

        // Set up video recording
        cycVideoBufRaw = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ);
        cycVideoBufJpeg = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ);
        cameraThread = new CameraThread(capCam, cycVideoBufRaw, settings.color);
        videoFileWriter = new VideoFileWriter(cycVideoBufJpeg, settings.storagePath);
        videoCompressorThread = new VideoCompressorThread(cycVideoBufRaw, cycVideoBufJpeg, settings.color, settings.jpgQuality);

        connect(cycVideoBufJpeg, SIGNAL(chunkReady(unsigned char*, int)), this, SLOT(onDrawFrame(unsigned char*, int)));

        // Start video running
        videoFileWriter->start();
        videoCompressorThread->start();
        cameraThread->start();

        eventTmr = new QTimer(this);
        eventTmr->setSingleShot(true);
        connect(eventTmr, SIGNAL(timeout()), this, SLOT(getNextEvent()));

        events = new EventContainer();
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
    delete []imBuf;
    delete []row_pointer[0];
    delete ui;
}

void VideoDialog::onDrawFrame(unsigned char*  jpegBuf, int logSize)
{
    ChunkAttrib chunkAttrib = *((ChunkAttrib*)(jpegBuf-sizeof(ChunkAttrib)-logSize-1));
    QString log = QString::fromAscii((char*)(jpegBuf - logSize-1));

    //---------------------------------------------------------------------
    // Decompress JPEG image to memory
    //

    // initialize JPEG
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, jpegBuf, chunkAttrib.chunkSize);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    for (int i=0; i<VIDEO_HEIGHT; i++)
    {
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
        memcpy((char*)imBuf + i*VIDEO_WIDTH*(color?3:1), row_pointer[0], VIDEO_WIDTH*(color?3:1));
    }

    // clean-up JPEG
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    //
    // done decompressing
    //---------------------------------------------------------------------

    QImage qImg((uchar*)imBuf, VIDEO_WIDTH, VIDEO_HEIGHT, QImage::Format_RGB888);
    ui->pixmapLabel->setPixmap(QPixmap::fromImage(qImg));
    qint64 elapsedTime = elapsedTimer.nsecsElapsed();

    //Write to to the logfile
    QTextStream logStream(&logFile);
    if(logSize && keepLog)
        logStream << "[" << elapsedTime/1000000000 << "s "
                  << (elapsedTime%1000000000)/1000000 << "ms]"
                  << log << "\n";
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
    isRec = arg;
    cycVideoBufJpeg->setIsRec(arg);
}

void VideoDialog::getNextEvent()
{
    Event *event = events->pop_front();

    if(event->getType() == EVENT_REMOVE)
        cameraThread->removeEvent(dynamic_cast<RemoveEvent*>(event));
    else
        cameraThread->addEvent(event);

    if(!events->empty())
    {
        Event *nextEvent = (*events)[0];
        eventTmr->start((nextEvent->getStart()+event->getDuration()+event->getDelay())*1000);
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
            eventTmr->start((*events)[0]->getStart()*1000);

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


    dc1394_camera_free_list(camList);

    videoAvailable = true;
    return true;
}

void VideoDialog::closeEvent(QCloseEvent *)
{
    window->toggleVideoDialogChecked(false);
}
