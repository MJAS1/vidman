#include <QMessageBox>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QGLFormat>
#include <memory>
#include "glvideowidget.h"
#include "settings.h"
#include "mainwindow.h"
#include "videodialog.h"
#include "ui_videodialog.h"
#include "config.h"
#include "common.h"

VideoDialog::VideoDialog(MainWindow *window, Camera& cam) :
    QDialog(window), ui(new Ui::VideoDialog), window_(window), cam_(cam)
{
    setWindowFlags(Qt::Window);
    ui->setupUi(this);

    connect(ui->aspectRatioSlider, SIGNAL(valueChanged(int)), this, SIGNAL(aspectRatioChanged(int)));
    /*Setup GLVideoWidget for drawing video frames. SwapInterval is used to sync
      trigger signals with screen refresh rate. */
    QGLFormat format;
    if(!format.hasOpenGL())
        std::cerr << "OpenGL not supported by window system. Cannot use vsync." << std::endl;

    Settings settings;
    format.setSwapInterval(settings.vsync);
    glVideoWidget_ = new GLVideoWidget(format, this);
    ui->verticalLayout->addWidget(glVideoWidget_, 1);

    initUI();
}

void VideoDialog::initUI()
{
    // Setup gain/shutter sliders
    if(!cam_.empty()) {
        ui->shutterSlider->setMinimum(SHUTTER_MIN_VAL);
        ui->shutterSlider->setMaximum(SHUTTER_MAX_VAL);
        ui->shutterSlider->setValue(cam_.getShutter() - SHUTTER_OFFSET);

        ui->gainSlider->setMinimum(GAIN_MIN_VAL);
        ui->gainSlider->setMaximum(GAIN_MAX_VAL);
        ui->gainSlider->setValue(cam_.getGain() - GAIN_OFFSET);

        ui->uvSlider->setMinimum(UV_MIN_VAL);
        ui->uvSlider->setMaximum(UV_MAX_VAL);

        ui->vrSlider->setMinimum(VR_MIN_VAL);
        ui->vrSlider->setMaximum(VR_MAX_VAL);
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
    painter.drawRect(glVideoWidget_->pos().x(), 0, glVideoWidget_->width()-1, glVideoWidget_->pos().y());
    painter.fillRect(glVideoWidget_->pos().x()+1, 1, glVideoWidget_->width()-2, glVideoWidget_->pos().y()-1, gradient);
}

void VideoDialog::onShutterChanged(int newVal)
{
    cam_.setShutter(newVal);
}

void VideoDialog::onGainChanged(int newVal)
{
    cam_.setGain(newVal);
}

void VideoDialog::onUVChanged(int newVal)
{
    cam_.setUV(newVal, ui->vrSlider->value());
}

void VideoDialog::onVRChanged(int newVal)
{
    cam_.setVR(newVal, ui->uvSlider->value());
}

void VideoDialog::closeEvent(QCloseEvent *)
{
    window_->toggleVideoDialogChecked(false);
}

void VideoDialog::updateFPS(int fps)
{
    ui->FPSLabel->setText(QString("FPS: %1").arg(fps));
}

void VideoDialog::onExternTrig(bool on)
{
    cam_.setExternTrigger(on);
}

MainWindow* VideoDialog::mainWindow() const
{
    return window_;
}

void VideoDialog::increaseAspectRatio()
{
    ui->aspectRatioSlider->setValue(ui->aspectRatioSlider->value()+3);
}

void VideoDialog::decreaseAspectRatio()
{
    ui->aspectRatioSlider->setValue(ui->aspectRatioSlider->value()-3);
}
