#ifndef MOTIONDIALOG_H
#define MOTIONDIALOG_H

#include <QDialog>
#include <opencv2/opencv.hpp>
#include "event.h"
#include "eventcontainer.h"

class MainWindow;

namespace Ui {
class MotionDialog;
}

class MotionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MotionDialog(MainWindow *parent = 0);
    ~MotionDialog();

public slots:
    void setPixmap(const QPixmap&);
    void onColorButtonToggled(bool);

signals:
    void changeColors(bool);

private:
    void closeEvent(QCloseEvent*);

    Ui::MotionDialog *ui;
    MainWindow* window_;
};

#endif // MOTIONDIALOG_H
