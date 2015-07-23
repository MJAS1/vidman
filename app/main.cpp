#include <QtGui/QApplication>
#include <QStyleFactory>
#include <mainwindow.h>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_X11InitThreads);
    QApplication a(argc, argv);

    QApplication::setStyle( QStyleFactory::create( "Fusion" ) );
    MainWindow w;
    w.show();
    
    return a.exec();
}
