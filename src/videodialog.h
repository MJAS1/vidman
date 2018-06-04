#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H

#include <QDialog>
#include <QTimer>
#include "settings.h"

class MainWindow;
class QGLContext;
class Camera;
class GLVideoWidget;

namespace Ui {
class VideoDialog;
}

/*!
 * VideoDialog contains the glVideoWidget, which is used to display video
 * frames. It also contains sliders to control camera shutter, gain etc.
 */
class VideoDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit VideoDialog(MainWindow *window, Camera &cam);
    ~VideoDialog();

    GLVideoWidget* glVideoWidget() const;

    // Returns the OpenGL context of the glVideoWidget child widget.
    QGLContext* context() const;

public slots:
    void onShutterChanged(int newVal);
    void onGainChanged(int newVal);
    void onUVChanged(int newVal);
    void onVRChanged(int newVal);
    void onExternTrig(bool on);
    void onAspectRatioSliderMoved(int videoWidth);
    void onNewFrame(unsigned char*);

signals:
    void aspectRatioChanged(int videoWidth);
    void windowClosed();

private:
    VideoDialog(const VideoDialog&);
    VideoDialog& operator=(const VideoDialog&);

    void initUI();
    void closeEvent(QCloseEvent *);
    void paintEvent(QPaintEvent*);

    Ui::VideoDialog*        ui;
    GLVideoWidget*          glVideoWidget_;
    Camera&                 cam_;
    Settings                settings_;
    int                     n_frames_;
    uint64_t                prevTimestamp_;
    int                     frameCnt_;
};

#endif // VIDEODIALOG_H
