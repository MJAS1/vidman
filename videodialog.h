#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H

#include <QDialog>
#include <QElapsedTimer>
#include <QFile>
#include <cv.h>
#include <highgui.h>
#include <jpeglib.h>
#include <dc1394/dc1394.h>
#include <QMainWindow>
#include "glvideowidget.h"
#include "cycdatabuffer.h"
#include "videofilewriter.h"
#include "camerathread.h"
#include "videocompressorthread.h"
#include "event.h"
#include "eventcontainer.h"
#include "eventreader.h"
#include "settings.h"

class MainWindow;

namespace Ui {
class VideoDialog;
}

class VideoDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit VideoDialog(MainWindow *window, QWidget *parent = 0);
    ~VideoDialog();
    volatile bool color;

    bool start(const QString& eventStr);
    void stop();
    void pause();
    void unpause();
    void setKeepLog(bool);
    void writeToLogFile(QString);
    void setTrigPort(int fd, PortType trigPort);
    void setFPS(int fps);

public slots:

    void onShutterChanged(int newVal);
    void onGainChanged(int newVal);
    void onUVChanged(int newVal);
    void onVRChanged(int newVal);

    void stopThreads();
    void toggleRecord(bool);

protected:
    void mouseDoubleClickEvent(QMouseEvent *e);
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
    int         eventDuration;

    EventContainer* events;
    GLVideoWidget*  glVideoWidget;

    QTimer *eventTmr;
    QElapsedTimer elapsedTimer;
    QFile logFile;

    qint64 time;

    Settings settings;
};

#endif // VIDEODIALOG_H
