#include <QtGui/QApplication>
#include <mainwindow.h>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_X11InitThreads);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
