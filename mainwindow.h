#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <cv.h>
#include <highgui.h>
#include <dc1394/dc1394.h>
#include "videodialog.h"
#include "event.h"
#include "eventreader.h"
#include "highlighter.h"
#include "timerwithpause.h"

namespace Ui {
class MainWindow;
}

enum State {
    STOPPED,
    PLAYING,
    PAUSED
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void toggleStart(bool);
    void toggleVideoDialogChecked(bool);
    
private slots:
    void onStart();
    void onStop();
    void onRec(bool);

    void onViewVideoDialog(bool);
    void onKeepLog(bool);

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
    void addRemoveEventId();
    void addRemoveEventType();
    void addImageObject();

    void updateTime();

    void closeEvent(QCloseEvent *e);

private:
    Ui::MainWindow*     ui;

    bool                load(const QString &f);
    bool                maybeSave();
    void                setCurrentFileName(const QString &fileName);
    void                pause();
    void                unpause();

    State               programState;

    cv::VideoCapture*   capCam;
    VideoDialog*        videoDialog;

    QTimer*             timeTmr;
    TimerWithPause      runningTime;
    QTime               time;
    QString             fileName;

    EventReader         eventReader;
    Highlighter*        highlighter;
};

#endif // MAINWINDOW_H
