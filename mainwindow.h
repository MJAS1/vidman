#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <dc1394/dc1394.h>
#include "videodialog.h"
#include "event.h"
#include "eventreader.h"
#include "highlighter.h"
#include "timerwithpause.h"
#include "outputdevice.h"

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
    qint64 getRunningTime() const;
    
private slots:
    void onStart();
    void onStop();
    void onRec(bool);

    void onViewVideoDialog(bool);
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

    Highlighter*        highlighter;
    State               programState;
    OutputDevice        *trigPort;

    VideoDialog*        videoDialog;
    TimerWithPause      runningTime;
    QTimer*             timeTmr;
    QString             fileName;
};

#endif // MAINWINDOW_H
