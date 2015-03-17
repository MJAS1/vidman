#include <QFileDialog>
#include <QMessageBox>
#include <QTextDocumentWriter>
#include <QToolButton>
#include <QStringList>
#include <QMenu>
#include <QCloseEvent>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "timerwithpause.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), programState_(STOPPED)
{
    ui->setupUi(this);
    videoDialog_ = new VideoDialog(this);
    videoDialog_->show();
    connect(&timeTmr_, SIGNAL(timeout()), this, SLOT(updateTime()));

    //ToolButton can't be assigned to toolbar in ui designer so it has to be done manually here.
    QMenu *menu = new QMenu(this);
    menu->addAction(ui->actionAddImageObject);
    menu->addAction(ui->actionAddVideoObject);
    menu->addSeparator();
    menu->addAction(ui->actionAddFadeInEvent);
    menu->addAction(ui->actionAddFadeOutEvent);
    menu->addAction(ui->actionAddFlipEvent);
    menu->addAction(ui->actionAddFreezeEvent);
    menu->addAction(ui->actionAddImageEvent);
    menu->addAction(ui->actionAddPlaybackEvent);
    menu->addAction(ui->actionAddRecordEvent);
    menu->addAction(ui->actionAddRotateEvent);
    menu->addAction(ui->actionAddTextEvent);
    menu->addAction(ui->actionAddZoomEvent);
    menu->addSeparator();
    menu->addAction(ui->actionRemoveEventId);
    menu->addAction(ui->actionRemoveEventType);

    QToolButton *toolBtn = new QToolButton(this);
    toolBtn->setMenu(menu);
    toolBtn->setIcon(QIcon::fromTheme("insert-object"));
    toolBtn->setPopupMode(QToolButton::InstantPopup);
    toolBtn->setShortcut(QString("Ctrl+e"));
    ui->toolBar2->addWidget(toolBtn);

    highlighter_ = new Highlighter(ui->textEdit->document());
}

MainWindow::~MainWindow()
{
    delete videoDialog_;
    delete ui;
}

void MainWindow::toggleStart(bool arg)
{
   ui->startButton->setEnabled(arg);
}

void MainWindow::toggleRec(bool arg)
{
   ui->recButton->setEnabled(arg);
}

void MainWindow::toggleStop(bool arg)
{
    ui->stopButton->setEnabled(arg);
}

void MainWindow::onStart()
{
    switch (programState_) {

        case STOPPED:

            if(videoDialog_->start(ui->textEdit->toPlainText()))
            {
                //ui->stopButton->setEnabled(true);
                runningTime_.restart();
                ui->startButton->setIcon(QIcon::fromTheme("media-playback-pause"));
                programState_ = PLAYING;
                timeTmr_.start(100);
            }
            break;

        case PLAYING:
            pause();
            break;

        case PAUSED:
            unpause();
            break;
    }

}

void MainWindow::onStop()
{
    if(programState_ == PLAYING || programState_ == PAUSED)
    {
        ui->startButton->toggle();
        ui->recButton->setChecked(false);
        videoDialog_->toggleRecord(false);

        QTime time(0, 0);
        ui->timeLbl->setText(time.toString(QString("hh:mm:ss")));
        timeTmr_.stop();

        programState_ = STOPPED;
        ui->startButton->setChecked(false);
        ui->startButton->setIcon(QIcon::fromTheme("media-playback-start"));
    }

    videoDialog_->stop();
    ui->statusLbl->clear();
}

void MainWindow::onRec(bool arg)
{
    videoDialog_->toggleRecord(arg);
}

void MainWindow::pause()
{
    timeTmr_.stop();
    runningTime_.pause();
    ui->startButton->setIcon(QIcon::fromTheme("media-playback-start"));
    videoDialog_->pause();
    programState_ = PAUSED;
}

void MainWindow::unpause()
{
    timeTmr_.start(100);
    runningTime_.resume();
    ui->startButton->setIcon(QIcon::fromTheme("media-playback-pause"));
    videoDialog_->unpause();
    programState_ = PLAYING;
}

void MainWindow::onViewVideoDialog(bool checked)
{
    if(checked)
        videoDialog_->show();

    else
        videoDialog_->close();
}

void MainWindow::onUpdateBackground()
{
    videoDialog_->updateBackground();
}

void MainWindow::onKeepLog(bool arg)
{
    videoDialog_->setKeepLog(arg);
}

void MainWindow::onSerialPort(bool arg)
{
    if(arg)
    {
        ui->actionParallelPort->setChecked(false);
        videoDialog_->setOutputDevice(OutputDevice::PORT_SERIAL);
    }
    else
        videoDialog_->setOutputDevice(OutputDevice::PORT_NULL);
}

void MainWindow::onParallelPort(bool arg)
{
    if(arg)
    {
        ui->actionSerialPort->setChecked(false);
        videoDialog_->setOutputDevice(OutputDevice::PORT_PARALLEL);
    }
    else
        videoDialog_->setOutputDevice(OutputDevice::PORT_NULL);
}

void MainWindow::updateTime()
{
    QTime time;
    time.setHMS(0, 0, 0);
    qint64 secsElapsed = runningTime_.nsecsElapsed()/1000000000;
    time = time.addSecs(secsElapsed);
    ui->timeLbl->setText(time.toString(QString("hh:mm:ss")));
}

void MainWindow::fileOpen()
{
    QString fn = QFileDialog::getOpenFileName(this, "Open File...",
                                              QString(), tr("MEG files (*.meg);;All Files (*)"));
    if (!fn.isEmpty())
        load(fn);
}

bool MainWindow::load(const QString &f)
{
    if (!QFile::exists(f))
        return false;
    QFile file(f);
    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    QString str = QString::fromLocal8Bit(data);

    ui->textEdit->setPlainText(str);
    ui->textEdit->document()->setModified(false);
    setCurrentFileName(f);


    return true;
}

bool MainWindow::maybeSave()
{
    if (!ui->textEdit->document()->isModified())
        return true;
    if (fileName_.startsWith(QLatin1String(":/")))
        return true;
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, "VideoManipulation",
                               "The document has been modified.\n"
                                  "Do you want to save your changes?",
                               QMessageBox::Save | QMessageBox::Discard
                               | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return fileSave();
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

bool MainWindow::fileSave()
{
    if (fileName_.isEmpty())
        return fileSaveAs();

    QTextDocumentWriter writer(fileName_);
    writer.setFormat("plaintext");
    bool success = writer.write(ui->textEdit->document());
    if (success)
    {
        ui->textEdit->document()->setModified(false);
    }
    return success;
}

bool MainWindow::fileSaveAs()
{
    QString fn = QFileDialog::getSaveFileName(this, tr("Save as..."),
                                              QString(), "MEG files (*.meg);;All Files (*)");
    if (fn.isEmpty())
        return false;
    if (!(fn.endsWith(".meg", Qt::CaseInsensitive)) && !(fn.endsWith(".txt", Qt::CaseInsensitive)))
        fn += ".meg"; // default

    setCurrentFileName(fn);
    return fileSave();
}

void MainWindow::setCurrentFileName(const QString &fileName)
{
    fileName_ = fileName;
    ui->textEdit->document()->setModified(false);

    QString shownName;
    if (fileName.isEmpty())
        shownName = "untitled.meg";
    else
        shownName = QFileInfo(fileName).fileName();

    setWindowTitle(tr("%1[*]").arg(shownName));
    setWindowModified(false);
}

void MainWindow::fileNew()
{
    if (maybeSave()) {
        ui->textEdit->clear();
        setCurrentFileName(QString());
    }
}

void MainWindow::addFlipEvent()
{
    QString str("Event: type=flip, start=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addFadeInEvent()
{
    QString str("Event: type=fade in, start=0, duration=5000, delay=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addFadeOutEvent()
{
    QString str("Event: type=fade out, start=0, duration=5000, delay=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addImageEvent()
{
    QString str("Event: type=image, start=0, x=0, y=0, image id=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addTextEvent()
{
    QString str("Event: type=text, start=0, x=0, y=0, color=black, text=");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addImageObject()
{
    QString str("ImageObject: id=0, filename=");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addVideoObject()
{
    QString str("VideoObject: id=0, length=2000");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addRotateEvent()
{
    QString str("Event: type=rotate, start=0, angle=90");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addFreezeEvent()
{
    QString str("Event: type=freeze, start=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addRemoveEventId()
{
    QString str("RemoveEvent: start=0, id=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addRemoveEventType()
{
    QString str("RemoveEvent: start=0, type=");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addZoomEvent()
{
    QString str("Event: type=zoom, start=0, scale=1.5, duration=2000");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addRecordEvent()
{
    QString str("Event: type=record, start=0, duration=2000, videoId=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addPlaybackEvent()
{
    QString str("Event: type=playback, start=0, duration=2000, videoId=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (maybeSave())
    {
        e->accept();
        videoDialog_->close();
    }
    else
    {
        e->ignore();
        return;
    }

    QApplication::quit();
}

void MainWindow::toggleVideoDialogChecked(bool arg)
{
    ui->viewVideoDialogAction->setChecked(arg);
}

qint64 MainWindow::getRunningTime()
{
    return runningTime_.nsecsElapsed();
}


TimerWithPause& MainWindow::getTimer()
{
    return runningTime_;
}

void MainWindow::setStatus(const QString &str)
{
    //QApplication::beep();
    ui->statusLbl->setText(str);
}