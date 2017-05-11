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
 * VideoDialog contains the glVideoWidget which is used to display the video. It
 * also contains the sliders to control camera shutter, gain etc.
 */
class VideoDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit VideoDialog(MainWindow *window, Camera &cam);
    ~VideoDialog();

    GLVideoWidget* glVideoWidget();

    //Returns the context of childwidget glVideoWidget
    QGLContext* context() const;

public slots:
    void updateFPS();
    void onShutterChanged(int newVal);
    void onGainChanged(int newVal);
    void onUVChanged(int newVal);
    void onVRChanged(int newVal);
    void onExternTrig(bool on);
    void onAspectRatioSliderMoved(int videoWidth);
    void increaseAspectRatio();
    void decreaseAspectRatio();
    void onDrawFrame();

signals:
    void aspectRatioChanged(int videoWidth);
    void setVideoDialogAction(bool);

protected:
    void paintEvent(QPaintEvent*);

private:
    VideoDialog(const VideoDialog&);
    VideoDialog& operator=(const VideoDialog&);

    void initUI();
    void closeEvent(QCloseEvent *);

    Ui::VideoDialog*        ui;
    GLVideoWidget*          glVideoWidget_;
    Camera&                 cam_;
    Settings                settings_;

    QTimer                  fpsTimer_;

    int                     n_frames_;
};

#endif // VIDEODIALOG_H
