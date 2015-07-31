#include <QFileDialog>
#include <QMessageBox>
#include <QTextDocumentWriter>
#include <QTextStream>
#include <QToolButton>
#include <QStringList>
#include <QMenu>
#include <QCloseEvent>
#include <QStatusBar>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <memory>
#include "eventreader.h"
#include "videodialog.h"
#include "highlighter.h"
#include "eventcontainer.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "timerwithpause.h"

using std::shared_ptr;
using std::unique_ptr;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), state_(STOPPED)
{
    ui->setupUi(this);
    videoDialog_ = new VideoDialog(this);
    videoDialog_->show();
    connect(&timeTmr_, SIGNAL(timeout()), this, SLOT(updateTime()));

    highlighter_ = new Highlighter(ui->textEdit->document());

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
    menu->addAction(ui->actionDeleteId);
    menu->addAction(ui->actionDeleteType);

    QToolButton *toolBtn = new QToolButton(this);
    toolBtn->setMenu(menu);
    toolBtn->setIcon(QIcon::fromTheme("insert-object"));
    toolBtn->setPopupMode(QToolButton::InstantPopup);
    toolBtn->setShortcut(QString("Ctrl+e"));

    ui->toolBar->addWidget(toolBtn);

    //Set status bar
    status_.setIndent(10);
    status_.setStyleSheet("QLabel { color: red;}");
    statusBar()->addWidget(&status_, 1);

    motionDetectorLabel_.setWindowTitle("Motion Detector");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::toggleStart(bool arg)
{
   ui->startButton->setEnabled(arg);
}

void MainWindow::toggleRecEnabled(bool arg)
{
   ui->recButton->setEnabled(arg);
}

void MainWindow::toggleRecChecked(bool arg)
{
    ui->recButton->setChecked(arg);
}

void MainWindow::toggleStop(bool arg)
{
    ui->stopButton->setEnabled(arg);
}

void MainWindow::onStart()
{
    if(!ui->recButton->isChecked())
        status_.clear();

    switch (state_) {
        case STOPPED: {

            if(logFile_.isActive() && !logFile_.open())
                setStatus(QString("Error creating log file " +
                                  logFile_.fileName() + ". " + logFile_.errorString()));

            //Create a StringList from the texteditor.
            QStringList strList = ui->textEdit->toPlainText().split("\n");
            strList.append("");

            //Read, create and store all the events from strList
            EventContainerPtr events(new EventContainer);
            EventReader eventReader;

            connect(&eventReader, SIGNAL(error(const QString&)), this, SLOT(setStatus(const QString&)));

            if(eventReader.loadEvents(strList, *events)) {
                eventsDuration_.setHMS(0, 0, 0);
                eventsDuration_ = eventsDuration_.addMSecs(events->getTotalDuration());
                videoDialog_->start(std::move(events));
                runningTime_.restart();
                ui->startButton->setIcon(QIcon::fromTheme("media-playback-pause"));
                state_ = PLAYING;
                timeTmr_.start(100);
            }
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
    if(state_ == PLAYING || state_ == PAUSED) {
        ui->timeLbl->setText(QString("00:00:00/00:00:00"));
        timeTmr_.stop();

        state_ = STOPPED;
        ui->startButton->setChecked(false);
        ui->startButton->setIcon(QIcon::fromTheme("media-playback-start"));

        logFile_.close();
    }

    ui->recButton->setChecked(false);
    videoDialog_->toggleRecEnabledord(false);

    videoDialog_->stop();

    status_.clear();
}

void MainWindow::onRec(bool arg)
{
    videoDialog_->toggleRecEnabledord(arg);
    if(arg)
        setStatus("Recording...");
    else
        status_.clear();
}

void MainWindow::pause()
{
    timeTmr_.stop();
    runningTime_.pause();
    ui->startButton->setIcon(QIcon::fromTheme("media-playback-start"));
    videoDialog_->pause();
    state_ = PAUSED;
}

void MainWindow::unpause()
{
    timeTmr_.start(100);
    runningTime_.resume();
    ui->startButton->setIcon(QIcon::fromTheme("media-playback-pause"));
    videoDialog_->unpause();
    state_ = PLAYING;
}

void MainWindow::onViewVideoDialog(bool checked)
{
    if(checked)
        videoDialog_->show();

    else
        videoDialog_->close();
}

void MainWindow::onViewMotionDetector(bool checked)
{
    if(checked)
        motionDetectorLabel_.show();
    else
        motionDetectorLabel_.close();
}

void MainWindow::onKeepLog(bool arg)
{
    logFile_.setActive(arg);
}

void MainWindow::onSerialPort(bool arg)
{
    if(arg) {
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
    QTime time(0, 0);
    qint64 msecsElapsed = runningTime_.nsecsElapsed()/1000000;
    time = time.addMSecs(msecsElapsed);

    ui->timeLbl->setText(time.toString(QString("hh:mm:ss"))+"/"+eventsDuration_.toString(QString("hh:mm:ss")));
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
    if (success) {
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
    QString str("event: type=flip, start=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addFadeInEvent()
{
    QString str("event: type=fade in, start=0, duration=5000, delay=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addFadeOutEvent()
{
    QString str("event: type=fade out, start=0, duration=5000, delay=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addImageEvent()
{
    QString str("event: type=image, start=0, x=0, y=0, objectId=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addTextEvent()
{
    QString str("event: type=text, start=0, x=0, y=0, color=black, text=");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addImageObject()
{
    QString str("object: type=image, id=0, filename=");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addVideoObject()
{
    QString str("object: type=video, id=0, length=2000");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addRotateEvent()
{
    QString str("event: type=rotate, start=0, angle=90");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addFreezeEvent()
{
    QString str("event: type=freeze, start=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addDeleteId()
{
    QString str("delete: start=0, id=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addDeleteType()
{
    QString str("delete: start=0, type=");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addZoomEvent()
{
    QString str("event: type=zoom, start=0, scale=1.5, duration=2000");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addRecordEvent()
{
    QString str("event: type=record, start=0, duration=2000, objectId=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addPlaybackEvent()
{
    QString str("event: type=playback, start=0, duration=2000, objectId=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (maybeSave()) {
        e->accept();
        videoDialog_->close();
    }
    else {
        e->ignore();
        return;
    }

    QApplication::quit();
}

void MainWindow::toggleVideoDialogChecked(bool arg)
{
    ui->viewVideoDialogAction->setChecked(arg);
}

void MainWindow::setStatus(const QString &str)
{
    //QApplication::beep();
    status_.setText(str);
    std::cerr << str.toStdString() << std::endl;
}

void MainWindow::writeToLog(const QString &str)
{
    qint64 elapsed = runningTime_.nsecsElapsed();
    QString log;
    QTextStream stream(&log);
    stream << "[" << elapsed/1000000000 << "s " << (elapsed%1000000000)/1000000 << "ms]" << str;
    logFile_ << log;
}

void MainWindow::updateMotionDetectorLabel(const QPixmap& pixmap)
{
    motionDetectorLabel_.setPixmap(pixmap);
}
