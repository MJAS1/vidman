#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H

#include <QDialog>
#include "outputdevice.h"
#include "settings.h"

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
    void onShutterChanged(int newVal);
    void onGainChanged(int newVal);
    void onUVChanged(int newVal);
    void onVRChanged(int newVal);
    void onExternTrig(bool on);
    void onAspectRatioSliderMoved(int videoWidth);
    void increaseAspectRatio();
    void decreaseAspectRatio();

signals:
    void drawFrame(unsigned char*);
    void aspectRatioChanged(int videoWidth);
    void outputDeviceChanged(OutputDevice::PortType);

protected:
    void paintEvent(QPaintEvent*);

private:
    void initUI();

    VideoDialog(const VideoDialog&);
    VideoDialog& operator=(const VideoDialog&);

    void closeEvent(QCloseEvent *);


    Ui::VideoDialog*        ui;
    MainWindow*             window_;
    GLVideoWidget*          glVideoWidget_;
    Camera&                 cam_;
    Settings                settings_;
};

#endif // VIDEODIALOG_H
