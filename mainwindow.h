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

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    
private slots:
    void onStart();
    void onStop();
    void onRec();

    void onViewVideoDialog(bool arg1);

    void fileNew();
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();

    void addFlipEvent();
    void addFadeInEvent();
    void addFadeOutEvent();
    void addImageEvent();
    void addTextEvent();

    void addImageObject();

    void updateTime();

    void closeEvent(QCloseEvent *e);

private:
    Ui::MainWindow*     ui;

    void                initVideo();

    bool                load(const QString &f);
    bool                maybeSave();
    void                setCurrentFileName(const QString &fileName);

    cv::VideoCapture*   capCam;
    dc1394_t*           dc1394Context;
    dc1394camera_t*     camera;
    VideoDialog*        videoDialog;

    QTimer*             timeTmr;
    QTime               time;
    QString             fileName;

    EventReader         eventReader;
    Highlighter*        highlighter;
};

#endif // MAINWINDOW_H
