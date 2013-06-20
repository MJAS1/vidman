#include <QTimer>
#include <QMessageBox>
#include "mainwindow.h"
#include "videodialog.h"
#include "ui_videodialog.h"
#include "settings.h"
#include "config.h"


VideoDialog::VideoDialog(int _cameraId, MainWindow *window, QWidget *parent) :
    QDialog(parent), window(window), ui(new Ui::VideoDialog), isRec(false)
{
    ui->setupUi(this);

    Settings settings;
    color = settings.color;

    // Allocate memory. Since we do not know whether the image is going to BW
    // or color, allocate for color since it requires more memory.
    imBuf = (char*)malloc(VIDEO_HEIGHT*VIDEO_WIDTH*3);
    if (!imBuf)
    {
        std::cerr << "Error allocating memory" << std::endl;
        exit(EXIT_FAILURE);
    }

    row_pointer[0] = (unsigned char *)malloc(VIDEO_WIDTH*3);
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
        videoFileWriter = new VideoFileWriter(cycVideoBufJpeg, settings.storagePath, _cameraId);
        videoCompressorThread = new VideoCompressorThread(cycVideoBufRaw, cycVideoBufJpeg, settings.color, settings.jpgQuality);

        connect(cycVideoBufJpeg, SIGNAL(chunkReady(unsigned char*)), this, SLOT(onDrawFrame(unsigned char*)));

        // Start video running
        videoFileWriter->start();
        videoCompressorThread->start();
        cameraThread->start();

        brightnessTmr = new QTimer();

        eventTmr = new QTimer;
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
    free(imBuf);
    free(row_pointer[0]);    
    delete ui;
}

void VideoDialog::onDrawFrame(unsigned char*  _jpegBuf)
{
    int 		i;
    ChunkAttrib chunkAttrib;

    chunkAttrib = *((ChunkAttrib*)(_jpegBuf-sizeof(ChunkAttrib)));

    //---------------------------------------------------------------------
    // Decompress JPEG image to memory
    //

    // initialize JPEG
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, _jpegBuf, chunkAttrib.chunkSize);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);


    for (i=0; i<VIDEO_HEIGHT; i++)
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

void VideoDialog::recordVideo()
{
    isRec = !isRec;
    cycVideoBufJpeg->setIsRec(isRec);
}

void VideoDialog::getNextEvent()
{
    Event *event = events->pop_front();
    cameraThread->addEvent(event);

    if(!events->empty())
    {
        Event *nextEvent = (*events)[0];
        eventTmr->start((nextEvent->getStart()+event->getDelay())*1000);
    }
}

bool VideoDialog::start(const QString eventStr)
{
    QStringList strList = eventStr.split("\n");
    strList.append("");
    EventReader eventReader;

    if(eventReader.loadEvents(strList, events))
    {
        if(!events->empty())
        {
            cameraThread->clearEvents();
            cameraThread->setEvents(true);
            eventTmr->start((*events)[0]->getStart()*1000);
        }

        return true;
    }

    else
        return false;


}

void VideoDialog::stop()
{
    cameraThread->setEvents(false);
    eventTmr->stop();
    events->clear();
}

void VideoDialog::onShutterChanged(int _newVal)
{
    dc1394error_t	err;

    if (!cameraThread)
    {
        return;
    }

    err = dc1394_set_register(camera, SHUTTER_ADDR, _newVal + SHUTTER_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Could not set shutter register" << std::endl;
        //abort();
    }
}

void VideoDialog::onGainChanged(int _newVal)
{
    dc1394error_t	err;

    if (!cameraThread)
    {
        return;
    }

    err = dc1394_set_register(camera, GAIN_ADDR, _newVal + GAIN_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Could not set gain register" << std::endl;
        //abort();
    }
}


void VideoDialog::onUVChanged(int _newVal)
{
    dc1394error_t	err;

    if (!cameraThread)
    {
        return;
    }

    // Since UV and VR live in the same register, we need to take care of both
    err = dc1394_set_register(camera, WHITEBALANCE_ADDR, _newVal * UV_REG_SHIFT + ui->vrSlider->value() + WHITEBALANCE_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Could not set white balance register" << std::endl;
        //abort();
    }
}


void VideoDialog::onVRChanged(int _newVal)
{
    dc1394error_t	err;

    if (!cameraThread)
    {
        return;
    }

    // Since UV and VR live in the same register, we need to take care of both
    err = dc1394_set_register(camera, WHITEBALANCE_ADDR, _newVal + UV_REG_SHIFT * ui->uvSlider->value() + WHITEBALANCE_OFFSET);

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
        return false;
    }


    dc1394camera_list_t*	camList;
    dc1394error_t			err;

    dc1394Context = dc1394_new();
    if(!dc1394Context)
    {
        std::cerr << "Cannot initialize!" << std::endl;
        return false;
    }

    err = dc1394_camera_enumerate(dc1394Context, &camList);
    if (err != DC1394_SUCCESS)
    {
        std::cerr << "Failed to enumerate cameras" << std::endl;
        return false;
    }

    camera = NULL;

    if (camList->num == 0)
    {
        std::cerr << "No cameras found" << std::endl;
        return false;
    }

    // use the first camera in the list
    camera = dc1394_camera_new(dc1394Context, camList->ids[0].guid);
    if (!camera)
    {
        std::cerr << "Failed to initialize camera with guid " << camList->ids[0].guid << std::endl;
        return false;
    }
    std::cout << "Using camera with GUID " << camera->guid << std::endl;


    dc1394_camera_free_list(camList);

    videoAvailable = true;
    return true;
}
