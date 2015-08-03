#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H

#include <QDialog>
#include "outputdevice.h"

class MainWindow;
class Camera;
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
    explicit VideoDialog(MainWindow *window, Camera &cam);
    ~VideoDialog();

    void updateFPS(int fps);
    MainWindow* mainWindow() const;

public slots:
    void setOutputDevice(OutputDevice::PortType portType);

    void onShutterChanged(int newVal);
    void onGainChanged(int newVal);
    void onUVChanged(int newVal);
    void onVRChanged(int newVal);
    void onAspectRatioChanged(int newVal);
    void onExternTrig(bool on);

    void onDrawFrame(unsigned char*);

protected:
    void paintEvent(QPaintEvent*);

private:

    VideoDialog(const VideoDialog&);
    VideoDialog& operator=(const VideoDialog&);

    void closeEvent(QCloseEvent *);


    Ui::VideoDialog*        ui;
    MainWindow*             window_;
    GLVideoWidget*          glVideoWidget_;
    Camera&                 cam_;
};

#endif // VIDEODIALOG_H
