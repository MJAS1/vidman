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
    typedef typename std::shared_ptr<EventContainer<Event*>> EventsPtr;

    explicit VideoDialog(MainWindow *window);
    ~VideoDialog();

    void start(EventsPtr events);
    void stop();
    void pause();
    void unpause();
    void setKeepLog(bool);
    void displayFPS(int fps);
    void updateBackground();

    LogFile& logFile();

public slots:
    void setOutputDevice(OutputDevice::PortType portType);
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

    int                     eventDuration_;

    Ui::VideoDialog         *ui;
    MainWindow*             window_;

    Settings                settings_;
    LogFile                 logFile_;

    CycDataBuffer*          cycVideoBufRaw_;
    CycDataBuffer*          cycVideoBufJpeg_;
    CameraThread*           cameraThread_;
    VideoFileWriter*        videoFileWriter_;
    VideoCompressorThread*  videoCompressorThread_;

    EventsPtr               events_;
    GLVideoWidget*          glVideoWidget_;

    QTimer                  eventTmr_;
    QElapsedTimer           elapsedTimer_;
    qint64                  time_;

    Camera                  cam_;
};

#endif // VIDEODIALOG_H
