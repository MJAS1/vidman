#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H

#include <QDialog>
#include <QElapsedTimer>
#include <QFile>
#include <cv.h>
#include <highgui.h>
#include <jpeglib.h>
#include <queue>
#include <dc1394/dc1394.h>
#include "cycdatabuffer.h"
#include "videofilewriter.h"
#include "camerathread.h"
#include "videocompressorthread.h"
#include "event.h"
#include "eventcontainer.h"
#include "eventreader.h"

class MainWindow;

namespace Ui {
class VideoDialog;
}

class VideoDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit VideoDialog(int cameraId, MainWindow *window, QWidget *parent = 0);
    ~VideoDialog();
    volatile bool color;

    bool start(const QString& eventStr);
    void stop();
    void setKeepLog(bool);

public slots:
    void onDrawFrame(unsigned char* jpegBuf, int logSize);

    void onShutterChanged(int newVal);
    void onGainChanged(int newVal);
    void onUVChanged(int newVal);
    void onVRChanged(int newVal);

    void stopThreads();
    void toggleRecord(bool);

private slots:
    void getNextEvent();

private:

    void closeEvent(QCloseEvent *);
    bool initVideo();

    Ui::VideoDialog *ui;
    MainWindow*     window;

    cv::VideoCapture*       capCam;
    dc1394camera_t*         camera;
    dc1394_t*               dc1394Context;

    CameraThread*       	cameraThread;
    CycDataBuffer*			cycVideoBufRaw;
    CycDataBuffer*			cycVideoBufJpeg;
    VideoFileWriter*		videoFileWriter;
    VideoCompressorThread*	videoCompressorThread;

    bool        isRec, videoAvailable, keepLog;
    char*		imBuf;

    // JPEG-related stuff
    struct		jpeg_decompress_struct cinfo;
    struct		jpeg_error_mgr jerr;
    JSAMPROW 	row_pointer[1];

    EventContainer* events;

    QTimer *eventTmr;
    QElapsedTimer elapsedTimer;
    QFile logFile;
};

#endif // VIDEODIALOG_H
