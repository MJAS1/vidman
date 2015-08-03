#include "motiondialog.h"
#include "ui_motiondialog.h"
#include "mainwindow.h"

MotionDialog::MotionDialog(MainWindow *parent) :
    QDialog(parent),
    ui(new Ui::MotionDialog), mainWindow_(parent)
{
    ui->setupUi(this);
}

MotionDialog::~MotionDialog()
{
    delete ui;
}

void MotionDialog::setPixmap(const QPixmap &p)
{
    ui->label->setPixmap(p);
}

void MotionDialog::onColorButtonToggled(bool arg)
{
    mainWindow_->motionDialogButtonClicked(arg);
}
