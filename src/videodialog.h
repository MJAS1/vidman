#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H

#include <QDialog>
#include <QTimer>
#include <QElapsedTimer>
#include <memory>
#include "eventcontainer.h"
#include "settings.h"
#include "outputdevice.h"
#include "camera.h"

class MainWindow;
class GLVideoWidget;
class CycDataBuffer;
class CameraThread;
class VideoFileWriter;
class VideoCompressorThread;

namespace Ui {
class VideoDialog;
}

class VideoDialog : public QDialog
{
    Q_OBJECT
    
public:
    typedef typename std::unique_ptr<EventContainer<Event*>> EventsPtr;

    explicit VideoDialog(MainWindow *window);
    ~VideoDialog();

    void start(EventsPtr events);
    void stop();
    void pause();
    void unpause();
    void displayFPS(int fps);
    void updateBackground();

    MainWindow* mainWindow() const;

public slots:
    void setOutputDevice(OutputDevice::PortType portType);
    void onShutterChanged(int newVal);
    void onGainChanged(int newVal);
    void onUVChanged(int newVal);
    void onVRChanged(int newVal);
    void onAspectRatioChanged(int newVal);
    void onExternTrig(bool on);

    void stopThreads();
    void toggleRecEnabledord(bool);

    void fileWriterError(const QString&);

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
