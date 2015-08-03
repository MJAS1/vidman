#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QLabel>
#include "camera.h"
#include "settings.h"
#include "logfile.h"
#include "eventcontainer.h"
#include "timerwithpause.h"

class VideoDialog;
class CycDataBuffer;
class CameraThread;
class VideoFileWriter;
class VideoCompressorThread;
class MotionDialog;
class Highlighter;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:

    enum State {
        STOPPED,
        PLAYING,
        PAUSED
    };

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void toggleVideoDialogChecked(bool);
    void writeToLog(const QString&);
    
public slots:
    void setStatus(const QString&);
    void fileWriterError(const QString&);
    
private slots:
    void onStartButton();
    void onStopButton();
    void onRecButton(bool);

    void onViewVideoDialog(bool);
    void onViewMotionDetector(bool);
    void onKeepLog(bool);
    void onSerialPort(bool);
    void onParallelPort(bool);

    void fileNew();
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();

    void addFlipEvent();
    void addFadeInEvent();
    void addFadeOutEvent();
    void addImageEvent();
    void addTextEvent();
    void addRotateEvent();
    void addFreezeEvent();
    void addDeleteId();
    void addDeleteType();
    void addImageObject();
    void addVideoObject();
    void addZoomEvent();
    void addRecordEvent();
    void addPlaybackEvent();

    void updateTime();
    void getNextEvent();

    void closeEvent(QCloseEvent *e);
    void stopThreads();

private:
    MainWindow(const MainWindow& other);
    MainWindow& operator=(const MainWindow& other);

    Ui::MainWindow*         ui;

    void                    initToolButton();
    void                    initVideo();

    bool                    load(const QString &f);
    bool                    maybeSave();
    void                    setCurrentFileName(const QString &fileName);

    void                    start();
    void                    pause();
    void                    unpause();

    int                     currentEventDuration_;

    Highlighter*            highlighter_;
    State                   state_;

    VideoDialog*            videoDialog_;
    MotionDialog*           motionDialog_;
    LogFile                 logFile_;
    QString                 fileName_;
    QLabel                  status_;
    QLabel                  motionDetectorLabel_;
    QTime                   eventsDuration_;
    Camera                  cam_;
    Settings                settings_;

    CycDataBuffer*          cycVideoBufRaw_;
    CycDataBuffer*          cycVideoBufJpeg_;
    CameraThread*           cameraThread_;
    VideoFileWriter*        videoFileWriter_;
    VideoCompressorThread*  videoCompressorThread_;

    QTimer                  eventTmr_;
    QTimer                  timeTmr_;
    TimerWithPause          runningTime_;

    EventContainer          events_;
    qint64                  time_;
};

#endif // MAINWINDOW_H
