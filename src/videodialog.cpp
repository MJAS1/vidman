#include <QGLFormat>
#include "glvideowidget.h"
#include "mainwindow.h"
#include "videodialog.h"
#include "ui_videodialog.h"
#include "config.h"
#include "common.h"

VideoDialog::VideoDialog(MainWindow *parent, Camera& cam) :
    QDialog(parent), ui(new Ui::VideoDialog), cam_(cam), n_frames_(0),
    prevTimestamp_(0), frameCnt_(0)
{
    setWindowFlags(Qt::Window);
    ui->setupUi(this);

    /*
     * Setup GLVideoWidget for drawing video frames. SwapInterval is used to
     * sync trigger signals with screen refresh rate.
     */
    QGLFormat format;
    if(!format.hasOpenGL())
        std::cerr << "OpenGL not supported by window system." << std::endl;
    format.setSwapInterval(settings_.vsync);
    glVideoWidget_ = new GLVideoWidget(format, this);
    ui->verticalLayout->addWidget(glVideoWidget_, 1);

    initUI();

    connect(ui->aspectRatioSlider, SIGNAL(valueChanged(int)), this,
            SLOT(onAspectRatioSliderMoved(int)));
    connect(glVideoWidget_, SIGNAL(pause()), parent,
            SLOT(onStartButton()));
    connect(this, SIGNAL(windowClosed()), parent,
            SLOT(onVideoDialogClosed()));

    ui->aspectRatioSlider->setValue(settings_.videoWidth);
}

void VideoDialog::initUI()
{
    // Setup gain/shutter sliders
    if(!cam_.empty()) {
        cam_.setShutter(settings_.shutter);
        ui->shutterSlider->setMinimum(SHUTTER_MIN_VAL);
        ui->shutterSlider->setMaximum(SHUTTER_MAX_VAL);
        ui->shutterSlider->setValue(settings_.shutter);

        cam_.setGain(settings_.gain);
        ui->gainSlider->setMinimum(GAIN_MIN_VAL);
        ui->gainSlider->setMaximum(GAIN_MAX_VAL);
        ui->gainSlider->setValue(settings_.gain);

        ui->uvSlider->setMinimum(UV_MIN_VAL);
        ui->uvSlider->setMaximum(UV_MAX_VAL);

        ui->vrSlider->setMinimum(VR_MIN_VAL);
        ui->vrSlider->setMaximum(VR_MAX_VAL);

        cam_.setWhiteBalance(settings_.whitebalance);
    }
    else {
        ui->shutterSlider->setEnabled(false);
        ui->gainSlider->setEnabled(false);
        ui->uvSlider->setEnabled(false);
        ui->vrSlider->setEnabled(false);
    }
}

VideoDialog::~VideoDialog()
{
    delete ui;
}

void VideoDialog::paintEvent(QPaintEvent *)
{
    //Paint the area under sliders and labels
    QPainter painter(this);
    painter.setPen(QColor(80, 80, 80));
    QLinearGradient gradient(QPoint(0, 0), QPoint(0, glVideoWidget_->pos().y()));
    gradient.setColorAt(0, QColor(150, 150, 150));
    gradient.setColorAt(0.5, QColor(140, 140, 140));
    gradient.setColorAt(1, QColor(80, 80, 80));
    painter.drawRect(glVideoWidget_->pos().x(), 0, glVideoWidget_->width()-1,
                     glVideoWidget_->pos().y());
    painter.fillRect(glVideoWidget_->pos().x()+1, 1, glVideoWidget_->width()-2,
                     glVideoWidget_->pos().y()-1, gradient);
}

void VideoDialog::onShutterChanged(int newVal)
{
    cam_.setShutter(newVal);
    settings_.setValue("video/shutter", cam_.getShutter() - SHUTTER_OFFSET);
}

void VideoDialog::onGainChanged(int newVal)
{
    cam_.setGain(newVal);
    settings_.setValue("video/gain", cam_.getGain() - GAIN_OFFSET);
}

void VideoDialog::onUVChanged(int newVal)
{
    cam_.setUV(newVal, ui->vrSlider->value());
    settings_.setValue("video/whiteBalance", cam_.getWhiteBalance());
}

void VideoDialog::onVRChanged(int newVal)
{
    cam_.setVR(newVal, ui->uvSlider->value());
    settings_.setValue("video/whiteBalance", cam_.getWhiteBalance());
}

void VideoDialog::closeEvent(QCloseEvent *)
{
    emit windowClosed();
}

void VideoDialog::onNewFrame(unsigned char* buf)
{
    ChunkAttrib chunkAttrib = *((ChunkAttrib*)(buf-sizeof(ChunkAttrib)));

    //Update and show FPS
    if(frameCnt_ == 10) {
        float fps = 1 / (float(chunkAttrib.timestamp - prevTimestamp_) / 1000);
        ui->FPSLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 2));
        frameCnt_ = 0;
    }

    prevTimestamp_ = chunkAttrib.timestamp;
    frameCnt_++;
}

void VideoDialog::onExternTrig(bool on)
{
    cam_.setExternTrigger(on);
}

void VideoDialog::onAspectRatioSliderMoved(int videoWidth)
{
    settings_.setValue("video/videoWidth", videoWidth);
    emit aspectRatioChanged(videoWidth);
}

QGLContext* VideoDialog::context() const
{
    return glVideoWidget_->context();
}

GLVideoWidget* VideoDialog::glVideoWidget() const
{
    return glVideoWidget_;
}
