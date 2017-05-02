#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H

#include <QDialog>
#include "outputdevice.h"
#include "settings.h"

class MainWindow;
class QGLContext;
class Camera;
class GLVideoWidget;
class CycDataBuffer;
class CameraThread;
class VideoFileWriter;
class VideoCompressorThread;

namespace Ui {
class VideoDialog;
}

/*!
 * VideoDialog contains the glVideoWidget which is used to display the video. It
 * also contains the sliders to control camera shutter, gain etc.
 */
class VideoDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit VideoDialog(MainWindow *window, Camera &cam);
    ~VideoDialog();

    void updateFPS(int fps);
    MainWindow* mainWindow() const;
    GLVideoWidget* glVideoWidget();

    //Returns the context of childwidget glVideoWidget
    QGLContext* context() const;

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

protected:
    void paintEvent(QPaintEvent*);

private:
    VideoDialog(const VideoDialog&);
    VideoDialog& operator=(const VideoDialog&);

    void initUI();
    void closeEvent(QCloseEvent *);

    Ui::VideoDialog*        ui;
    MainWindow*             window_;
    GLVideoWidget*          glVideoWidget_;
    Camera&                 cam_;
    Settings                settings_;
};

#endif // VIDEODIALOG_H
