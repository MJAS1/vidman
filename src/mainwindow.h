/*
 * mainwindow.h
 *
 * Author: Manu Sutela
 * Copyright (C) 2018 Aalto University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QLabel>
#include <QThread>
#include <QElapsedTimer>

#include "camera.h"
#include "outputdevice.h"
#include "glworker.h"
#include "settings.h"
#include "logfile.h"
#include "eventcontainer.h"
#include "timerwithpause.h"
#include "cameraworker.h"

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

/*!
 * This is the main window of the program. It contains the text editor, play
 * and stop buttons, file menu, etc.
 */
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

    void toggleMotionDialogChecked(bool);
    void writeToLog(const QString&);

signals:
    void outputDeviceChanged(OutputDevice::PortType);
    void stopCameraWorker();
    
public slots:
    void onVideoDialogClosed();
    void setStatus(const QString&);
    void fileWriterError(const QString&);
    void pause();
    
private slots:
    void onStartButton();
    void onStopButton();
    void onRecButton(bool);

    void onViewVideoDialog(bool);
    void onViewMotionDetector(bool);
    void onKeepLog(bool);
    void onSerialPort(bool);
    void onParallelPort(bool);
    void onVBlank(const ChunkAttrib*);

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
    void addMotionDetectorEvent();

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
    void                    unpause();

    int                     currentEventDuration_;

    qint64                  time_;

    Highlighter*            highlighter_;
    State                   state_;

    Camera                  cam_;
    VideoDialog*            videoDialog_;
    MotionDialog*           motionDialog_;
    LogFile                 logFile_;
    QString                 fileName_;
    QLabel                  status_;
    QLabel                  motionDetectorLabel_;
    QTime                   eventsDuration_;
    Settings                settings_;

    CycDataBuffer*          cycVideoBufRaw_;
    CycDataBuffer*          cycVideoBufJpeg_;
    QThread*                workerThread_;
    VideoFileWriter*        videoFileWriter_;
    VideoCompressorThread*  videoCompressorThread_;
    CameraWorker            cameraWorker_;
    GLWorker                glworker_;

    QTimer                  eventTmr_;
    QTimer                  timeTmr_;
    TimerWithPause          runningTime_;

    EventContainer          events_;
    OutputDevice            trigPort_;
};

#endif // MAINWINDOW_H
