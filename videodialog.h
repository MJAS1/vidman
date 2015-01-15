#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H

#include <QDialog>
#include <QElapsedTimer>
#include <QFile>
#include <opencv/cv.h>
#include <opencv/highgui.h>
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
#include "outputdevice.h"
#include "camera.h"
#include "logfile.h"

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

    bool start(const QString& eventStr);
    void stop();
    void pause();
    void unpause();
    void setKeepLog(bool);
    void setFPS(int fps);
    void updateBackground();

public slots:

    bool setOutputDevice(OutputDevice::PortType portType);
    void onShutterChanged(int newVal);
    void onGainChanged(int newVal);
    void onUVChanged(int newVal);
    void onVRChanged(int newVal);
    void onWidthChanged(int newVal);
    void onExternTrig(bool on);

    void stopThreads();
    void toggleRecord(bool);

private slots:
    void getNextEvent();

private:
    VideoDialog(const VideoDialog&);
    VideoDialog& operator=(const VideoDialog&);

    void closeEvent(QCloseEvent *);

    Ui::VideoDialog *ui;
    MainWindow*     window;

    Camera                  camera;

    CameraThread*       	cameraThread;
    CycDataBuffer*			cycVideoBufRaw;
    CycDataBuffer*			cycVideoBufJpeg;
    VideoFileWriter*		videoFileWriter;
    VideoCompressorThread*	videoCompressorThread;

    bool        videoAvailable;
    int         eventDuration;

    EventContainer<Event*> events;
    GLVideoWidget*  glVideoWidget;

    QTimer eventTmr;
    QElapsedTimer elapsedTimer;

    qint64 time;

    Settings settings;
    LogFile	logFile;
};

#endif // VIDEODIALOG_H
