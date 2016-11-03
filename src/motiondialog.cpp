#include <QPainter>
#include "motiondialog.h"
#include "ui_motiondialog.h"
#include "mainwindow.h"

MotionDialog::MotionDialog(MainWindow *parent) :
    QDialog(parent),
    ui(new Ui::MotionDialog), window_(parent)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);
}

MotionDialog::~MotionDialog()
{
    delete ui;
}

void MotionDialog::setPixmap(const QPixmap &p)
{
    ui->label->setPixmap(p);
}

void MotionDialog::closeEvent(QCloseEvent *)
{
    window_->toggleMotionDialogChecked(false);
}
