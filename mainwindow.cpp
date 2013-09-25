#include <QFileDialog>
#include <QMessageBox>
#include <QTextDocumentWriter>
#include <QToolButton>
#include <QStringList>
#include <QMenu>
#include <QCloseEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), videoDialog(new VideoDialog(this)), timeTmr(new QTimer(this))
{
    ui->setupUi(this);
    videoDialog->show();
    connect(timeTmr, SIGNAL(timeout()), this, SLOT(updateTime()));

    //For some reason ToolButton can't be assigned to toolbar in ui designer so it has to be done manually in the code.
    QMenu *menu = new QMenu(this);
    menu->addAction(ui->actionAddImageObject);
    menu->addSeparator();
    menu->addAction(ui->actionAddFadeInEvent);
    menu->addAction(ui->actionAddFadeOutEvent);
    menu->addAction(ui->actionAddFlipEvent);
    menu->addAction(ui->actionAddFreezeEvent);
    menu->addAction(ui->actionAddImageEvent);
    menu->addAction(ui->actionAddRotateEvent);
    menu->addAction(ui->actionAddTextEvent);
    menu->addSeparator();
    menu->addAction(ui->actionRemoveEventId);
    menu->addAction(ui->actionRemoveEventType);

    QToolButton *toolBtn = new QToolButton(this);
    toolBtn->setMenu(menu);
    toolBtn->setIcon(QIcon::fromTheme("insert-object"));
    toolBtn->setPopupMode(QToolButton::InstantPopup);
    toolBtn->setShortcut(QString("Ctrl+e"));
    ui->toolBar2->addWidget(toolBtn);

    highlighter = new Highlighter(ui->textEdit->document());

}

MainWindow::~MainWindow()
{
    delete videoDialog;
    delete ui;
}

void MainWindow::toggleStart(bool arg)
{
   ui->startButton->setEnabled(arg);
}

void MainWindow::onStart()
{
    if(videoDialog->start(ui->textEdit->toPlainText()))
    {
        ui->startButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
        timeTmr->start(1000);
    }
    else
    {
        ui->startButton->toggle();
    }
}

void MainWindow::onStop()
{

    ui->startButton->toggle();
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    ui->recButton->setChecked(false);

    videoDialog->toggleRecord(false);
    videoDialog->stop();

    time.setHMS(0, 0, 0);
    ui->timeLbl->setText(time.toString(QString("hh:mm:ss")));
    timeTmr->stop();
}

void MainWindow::onRec(bool arg)
{
    videoDialog->toggleRecord(arg);
}


void MainWindow::onViewVideoDialog(bool checked)
{
    if(checked)
        videoDialog->show();

    else
        videoDialog->close();
}

void MainWindow::onKeepLog(bool arg)
{
    videoDialog->setKeepLog(arg);
}

void MainWindow::updateTime()
{
    time = time.addSecs(1);
    ui->timeLbl->setText(time.toString(QString("hh:mm:ss")));
}

void MainWindow::fileOpen()
{
    QString fn = QFileDialog::getOpenFileName(this, "Open File...",
                                              QString(), tr("MEG files (*.meg);;All Files (*)"));
    if (!fn.isEmpty())
        load(fn);
}

bool MainWindow::load(const QString &f)
{
    if (!QFile::exists(f))
        return false;
    QFile file(f);
    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    QString str = QString::fromLocal8Bit(data);

    ui->textEdit->setPlainText(str);
    ui->textEdit->document()->setModified(false);
    setCurrentFileName(f);


    return true;
}

bool MainWindow::maybeSave()
{
    if (!ui->textEdit->document()->isModified())
        return true;
    if (fileName.startsWith(QLatin1String(":/")))
        return true;
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, "VideoManipulation",
                               "The document has been modified.\n"
                                  "Do you want to save your changes?",
                               QMessageBox::Save | QMessageBox::Discard
                               | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return fileSave();
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

bool MainWindow::fileSave()
{
    if (fileName.isEmpty())
        return fileSaveAs();

    QTextDocumentWriter writer(fileName);
    writer.setFormat("plaintext");
    bool success = writer.write(ui->textEdit->document());
    if (success)
    {
        ui->textEdit->document()->setModified(false);
    }
    return success;
}

bool MainWindow::fileSaveAs()
{
    QString fn = QFileDialog::getSaveFileName(this, tr("Save as..."),
                                              QString(), "MEG files (*.meg);;All Files (*)");
    if (fn.isEmpty())
        return false;
    if (!(fn.endsWith(".meg", Qt::CaseInsensitive)) && !(fn.endsWith(".txt", Qt::CaseInsensitive)))
        fn += ".meg"; // default

    setCurrentFileName(fn);
    return fileSave();
}

void MainWindow::setCurrentFileName(const QString &fileName)
{
    this->fileName = fileName;
    ui->textEdit->document()->setModified(false);

    QString shownName;
    if (fileName.isEmpty())
        shownName = "untitled.meg";
    else
        shownName = QFileInfo(fileName).fileName();

    setWindowTitle(tr("%1[*]").arg(shownName));
    setWindowModified(false);
}

void MainWindow::fileNew()
{
    if (maybeSave()) {
        ui->textEdit->clear();
        setCurrentFileName(QString());
    }
}

void MainWindow::addFlipEvent()
{
    QString str("#Event: type=flip, start=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addFadeInEvent()
{
    QString str("#Event: type=fade in, start=0, duration=5, delay=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addFadeOutEvent()
{
    QString str("#Event: type=fade out, start=0, duration=5, delay=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addImageEvent()
{
    QString str("#Event: type=image, start=0, x=0, y=0, image id=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addTextEvent()
{
    QString str("#Event: type=text, start=0, x=0, y=0, color=black, text=");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addImageObject()
{
    QString str("#ImageObject: id=0, filename=");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addRotateEvent()
{
    QString str("#Event: type=rotate, start=0, angle=90");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addFreezeEvent()
{
    QString str("#Event: type=freeze, start=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addRemoveEventId()
{
    QString str("#RemoveEvent: start=0, id=0");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::addRemoveEventType()
{
    QString str("#RemoveEvent: start=0, type=");
    ui->textEdit->insertPlainText(str);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (maybeSave())
    {
        e->accept();
        videoDialog->close();
    }
    else
        e->ignore();
}

void MainWindow::toggleVideoDialogChecked(bool arg)
{
    ui->viewVideoDialogAction->setChecked(arg);
}
