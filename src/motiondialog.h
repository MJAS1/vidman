#ifndef MOTIONDIALOG_H
#define MOTIONDIALOG_H

#include <QDialog>

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

    void setPixmap(const QPixmap&);

public slots:
    void onColorButtonToggled(bool);

private:
    Ui::MotionDialog *ui;
    MainWindow* mainWindow_;
};

#endif // MOTIONDIALOG_H
