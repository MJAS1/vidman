#ifndef MOTIONDIALOG_H
#define MOTIONDIALOG_H

#include <QDialog>

class MainWindow;

namespace Ui {
class MotionDialog;
}

/*!
 * MotionDialog is only visible when checked in the file menu of MainWindow.
 * It is used to display the 'differential image' produced by a
 * MotionDetectorEvent, which can be in turn used to adjust the threshold value
 * for motion detection.
 */
class MotionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MotionDialog(MainWindow *parent = 0);
    ~MotionDialog();

public slots:
    void setPixmap(const QPixmap&);

private:
    MotionDialog(const MotionDialog&);
    MotionDialog& operator=(const MotionDialog&);

    void closeEvent(QCloseEvent*);

    Ui::MotionDialog *ui;
    MainWindow* window_;
};

#endif // MOTIONDIALOG_H
