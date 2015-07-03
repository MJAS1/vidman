#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QLabel>
#include "videodialog.h"
#include "event.h"
#include "eventreader.h"
#include "highlighter.h"
#include "timerwithpause.h"
#include "outputdevice.h"

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

    void toggleStart(bool);
    void toggleRecEnabled(bool);
    void toggleRecChecked(bool);
    void toggleStop(bool);
    void toggleVideoDialogChecked(bool);
    void writeToLog(const QString&);
    
public slots:
    void setStatus(const QString&);
    void updateMotionDetectorLabel(std::shared_ptr<QPixmap>);
    
private slots:
    void onStart();
    void onStop();
    void onRec(bool);

    void onViewVideoDialog(bool);
    void onViewMotionDetector(bool);
    void onKeepLog(bool);
    void onSerialPort(bool);
    void onParallelPort(bool);
    void onUpdateBackground();

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

    void closeEvent(QCloseEvent *e);

private:
    MainWindow(const MainWindow& other);
    MainWindow& operator=(const MainWindow& other);

    Ui::MainWindow*     ui;

    bool                load(const QString &f);
    bool                maybeSave();
    void                setCurrentFileName(const QString &fileName);
    void                pause();
    void                unpause();

    Highlighter*        highlighter_;
    State               state_;

    VideoDialog*        videoDialog_;
    TimerWithPause      runningTime_;
    LogFile             logFile_;
    QTimer              timeTmr_;
    QString             fileName_;
    QLabel              status_;
    QLabel              motionDetectorLabel_;
    QTime               eventsDuration_;
};

#endif // MAINWINDOW_H
