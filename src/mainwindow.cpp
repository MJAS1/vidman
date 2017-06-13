#include <QFileDialog>
#include <QMessageBox>
#include <QTextDocumentWriter>
#include <QTextStream>
#include <QToolButton>
#include <QStringList>
#include <QMenu>
#include <QCloseEvent>
#include <QStatusBar>
#include <QDebug>
#include "glvideowidget.h"
#include "config.h"
#include "cycdatabuffer.h"
#include "videocompressorthread.h"
#include "motiondialog.h"
#include "eventparser.h"
#include "videodialog.h"
#include "videofilewriter.h"
#include "highlighter.h"
#include "eventcontainer.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "timerwithpause.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), time_(0), state_(STOPPED),
    videoDialog_(new VideoDialog(this, cam_)),
    cycVideoBufRaw_(new CycDataBuffer(CIRC_VIDEO_BUFF_SZ, this)),
    cycVideoBufJpeg_(new CycDataBuffer(CIRC_VIDEO_BUFF_SZ, this)),
    workerThread_(new QThread(this)),
    videoFileWriter_(new VideoFileWriter(cycVideoBufJpeg_,
                                         settings_.storagePath, this)),
    videoCompressorThread_(new VideoCompressorThread(cycVideoBufRaw_,
                                                     cycVideoBufJpeg_,
                                                     settings_.jpgQuality,
                                                     this)),
    cameraWorker_(cycVideoBufRaw_, cam_),
    glworker_(videoDialog_->glVideoWidget())
{
    //Order of function calls in constructor is important.
    ui->setupUi(this);
    motionDialog_ = new MotionDialog(this);
    connect(&timeTmr_, SIGNAL(timeout()), this, SLOT(updateTime()));
    connect(ui->viewMotionDetectorAction, SIGNAL(triggered(bool)),
            &cameraWorker_, SLOT(motionDialogToggled(bool)));

    qRegisterMetaType<OutputDevice::PortType>("OutputDevice::PortType");
    connect(this, SIGNAL(outputDeviceChanged(OutputDevice::PortType)),
            &trigPort_, SLOT(open(OutputDevice::PortType)));
    connect(videoDialog_, SIGNAL(aspectRatioChanged(int)), &glworker_,
            SLOT(onAspectRatioChanged(int)));
    connect(videoDialog_->glVideoWidget(), SIGNAL(resize(int,int)),
            &glworker_, SLOT(resizeGL(int,int)));

    /* Slots connected to vblank() will be synced with the screen refresh rate
     * as long as vsync is on. */
    connect(&glworker_, SIGNAL(vblank(const ChunkAttrib*)),
            this, SLOT(onVBlank(const ChunkAttrib*)), Qt::DirectConnection);
    connect(&glworker_, SIGNAL(vblank(const ChunkAttrib*)),
            &cameraWorker_, SLOT(captureFrame()), Qt::DirectConnection);

    trigPort_.moveToThread(workerThread_);
    logFile_.moveToThread(workerThread_);

    initToolButton();
    initVideo();

    videoDialog_->show();

    //Set status bar
    status_.setIndent(10);
    status_.setStyleSheet("QLabel { color: red;}");
    statusBar()->addWidget(&status_, 1);

    highlighter_ = new Highlighter(ui->textEdit->document());
    eventTmr_.setTimerType(Qt::PreciseTimer);
}

MainWindow::~MainWindow()
{
    if(!cam_.empty())
        stopThreads();
    delete ui;
}

void MainWindow::stopThreads()
{
    // The piece of code stopping the threads should execute fast enough,
    // otherwise cycVideoBufRaw or cycVideoBufJpeg buffer might overflow. The
    // order of stopping the threads is important.
    videoFileWriter_->stop();
    videoCompressorThread_->stop();
    glworker_.stop();
    workerThread_->quit();
    workerThread_->wait();
}


void MainWindow::initToolButton()
{
    //ToolButton can't be assigned to toolbar in ui designer so it has to be
    //done manually here.
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
    menu->addAction(ui->actionAddMotionDetectorEvent);
    menu->addSeparator();
    menu->addAction(ui->actionDeleteId);
    menu->addAction(ui->actionDeleteType);

    QToolButton *toolBtn = new QToolButton(this);
    toolBtn->setMenu(menu);
    toolBtn->setIcon(QIcon(":/img/add.svg"));
    toolBtn->setPopupMode(QToolButton::InstantPopup);
    toolBtn->setShortcut(QString("Ctrl+e"));

    ui->toolBar->addWidget(toolBtn);
}

void MainWindow::initVideo()
{
    if(!cam_.empty()) {
        // Set up video recording
        connect(videoFileWriter_, SIGNAL(error(const QString&)), this,
                SLOT(fileWriterError(const QString&)));
        connect(cycVideoBufRaw_, SIGNAL(chunkReady(unsigned char*)),
                videoDialog_, SLOT(onNewFrame(unsigned char*)));
        connect(cycVideoBufRaw_, SIGNAL(chunkReady(unsigned char*)), &glworker_,
                SLOT(onDrawFrame(unsigned char*)));
        connect(&cameraWorker_, SIGNAL(motionPixmapReady(const QPixmap&)),
                motionDialog_, SLOT(setPixmap(const QPixmap&)));

        // Start video running
        videoFileWriter_->start();
        videoCompressorThread_->start();
/* QGLContext::moveToThread() was introduced in Qt5 and is necessary to
 * enable OpenGL in a different thread. */
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        videoDialog_->context()->moveToThread(workerThread_);
#endif
        cameraWorker_.moveToThread(workerThread_);
        workerThread_->start();
        glworker_.moveToThread(workerThread_);
        glworker_.onAspectRatioChanged(settings_.videoWidth);
        glworker_.start();
        /* First frame needs to be captured manually here, the rest are handled
         * by connection to glworker vblank(). */
        cameraWorker_.captureFrame();

        //Setup event handling
        eventTmr_.setSingleShot(true);
        connect(&eventTmr_, SIGNAL(timeout()), this, SLOT(getNextEvent()));
    }
    else {
        setStatus(QString("Couldn't initialize video."));
        ui->startButton->setEnabled(false);
        ui->recButton->setEnabled(false);
        ui->stopButton->setEnabled(false);
    }
}

void MainWindow::getNextEvent()
{
    int delay = events_[0]->getDelay();
    cameraWorker_.addEvent(events_.pop_front());

    //Calculate the start time of the next event
    if(!events_.empty()) {
        time_ = runningTime_.msecsElapsed();
        currentEventDuration_ = (events_[0]->getStart()+delay);
        eventTmr_.start(currentEventDuration_);
    }
}

void MainWindow::onStartButton()
{
    if(!ui->recButton->isChecked())
        status_.clear();

    switch (state_) {
    case STOPPED:
        start();
        break;

    case PLAYING:
        pause();
        break;

    case PAUSED:
        unpause();
        break;
    }
}

void MainWindow::onStopButton()
{
    if(state_ == PLAYING || state_ == PAUSED) {
        ui->timeLbl->setText(QString("00:00:00/00:00:00"));
        timeTmr_.stop();

        state_ = STOPPED;
        ui->startButton->setChecked(false);
        ui->startButton->setIcon(QIcon(":/img/media-playback-start.svg"));

        logFile_.close();
    }

    ui->recButton->setChecked(false);
    cycVideoBufJpeg_->setIsRec(false);
    cameraWorker_.clearEvents();
    eventTmr_.stop();
    events_.clear();

    status_.clear();
}

void MainWindow::onRecButton(bool arg)
{
    cycVideoBufJpeg_->setIsRec(arg);
    if(arg)
        setStatus("Recording...");
    else
        status_.clear();
}

void MainWindow::start()
{
    if(logFile_.isActive() && !logFile_.open())
        setStatus(QString("Error creating log file " +
                          logFile_.fileName() + ". " + logFile_.errorString()));

    //Create a StringList from the texteditor.
    QStringList strList = ui->textEdit->toPlainText().split("\n");
    strList.append("");

    //Read, create and store all the events from strList
    EventParser eventParser;
    connect(&eventParser, SIGNAL(error(const QString&)), this,
            SLOT(setStatus(const QString&)));

    if(eventParser.loadEvents(strList, events_, this)) {
        eventsDuration_.setHMS(0, 0, 0);
        eventsDuration_ = eventsDuration_.addMSecs(events_.getTotalDuration());

        ui->startButton->setIcon(QIcon(":/img/media-playback-pause.svg"));
        state_ = PLAYING;
        if(!events_.empty()) {
            eventTmr_.start(events_[0]->getStart());
            time_ = 0;
        }

        runningTime_.restart();
        timeTmr_.start(100);
    }
}

void MainWindow::pause()
{
    timeTmr_.stop();
    eventTmr_.stop();
    runningTime_.pause();
    cameraWorker_.pause();
    time_ = runningTime_.msecsElapsed()-time_;

    ui->startButton->setIcon(QIcon(":/img/media-playback-start.svg"));
    state_ = PAUSED;
}

void MainWindow::unpause()
{
    timeTmr_.start(100);
    runningTime_.resume();
    if(!events_.empty()) {
        currentEventDuration_ = currentEventDuration_ - time_;
        eventTmr_.start(currentEventDuration_);
    }
    time_ = runningTime_.msecsElapsed();
    cameraWorker_.unpause();

    ui->startButton->setIcon(QIcon::fromTheme(":/img/media-playback-pause.svg"));
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
        motionDialog_->show();
    else
        motionDialog_->hide();
}

void MainWindow::onKeepLog(bool arg)
{
    logFile_.setActive(arg);
}

void MainWindow::onSerialPort(bool arg)
{
    if(arg) {
        ui->actionParallelPort->setChecked(false);
        emit outputDeviceChanged(OutputDevice::PORT_SERIAL);
    }
    else
        emit outputDeviceChanged(OutputDevice::PORT_NULL);
}

void MainWindow::onParallelPort(bool arg)
{
    if(arg)
    {
        ui->actionSerialPort->setChecked(false);
        emit outputDeviceChanged(OutputDevice::PORT_PARALLEL);
    }
    else
        emit outputDeviceChanged(OutputDevice::PORT_NULL);
}

void MainWindow::updateTime()
{
    QTime time(0, 0);
    qint64 msecsElapsed = runningTime_.msecsElapsed();
    time = time.addMSecs(msecsElapsed);

    ui->timeLbl->setText(time.toString(QString("hh:mm:ss"))
                         +"/"+eventsDuration_.toString(QString("hh:mm:ss")));
}

void MainWindow::fileOpen()
{
    QString fn = QFileDialog::getOpenFileName(this, "Open File...", QString(),
                                              tr("MEG files (*.meg);;All Files (*)"));
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
    ret = QMessageBox::warning(this, "VidMan",
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
    QString fn = QFileDialog::getSaveFileName(this, tr("Save as..."), QString(),
                                              "MEG files (*.meg);;All Files (*)");
    if (fn.isEmpty())
        return false;
    if (!(fn.endsWith(".meg", Qt::CaseInsensitive)) && !(fn.endsWith(".txt",Qt::CaseInsensitive)))
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
    QString str("event: type=fadein, start=0, duration=5000, delay=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addFadeOutEvent()
{
    QString str("event: type=fadeout, start=0, duration=5000, delay=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addImageEvent()
{
    QString str("event: type=image, start=0, x=0, y=0, objectId=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addTextEvent()
{
    QString str("event: type=text, start=0, x=0, y=0, color=black, string=");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addImageObject()
{
    QString str("object: type=image, id=0, filename=");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addVideoObject()
{
    QString str("object: type=video, id=0, duration=2000");
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
    QString str("event: type=record, start=0, duration=2000, delay=2000,"
                "objectId=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addPlaybackEvent()
{
    QString str("event: type=playback, start=0, duration=2000, objectId=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addMotionDetectorEvent()
{
    QString str("event: type=detectmotion, target=500, tolerance=50, "
                "trigCode=1, successcode=2, failcode=3");
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

void MainWindow::toggleMotionDialogChecked(bool arg)
{
    ui->viewMotionDetectorAction->setChecked(arg);
    emit ui->viewMotionDetectorAction->triggered(arg);
}

void MainWindow::setStatus(const QString &str)
{
    //QApplication::beep();
    status_.setText(str);
    std::cerr << str.toStdString() << std::endl;
}

void MainWindow::writeToLog(const QString &str)
{
    qint64 elapsed = runningTime_.msecsElapsed();
    QString log;
    QTextStream stream(&log);
    stream << "[" << elapsed/1000 << "s " << (elapsed%1000) << "ms]" << str;
    logFile_ << log;
}

void MainWindow::fileWriterError(const QString &str)
{
    cycVideoBufJpeg_->setIsRec(false);
    ui->recButton->setEnabled(false);
    setStatus(str);
}

void MainWindow::onVBlank(const ChunkAttrib *chunkAttrib)
{
    trigPort_.writeData(chunkAttrib->trigCode);
    if(strlen(chunkAttrib->log))
        writeToLog(chunkAttrib->log);
}
