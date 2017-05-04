#ifndef GLWORKER_H
#define GLWORKER_H

#include <QObject>
#include <QGLShaderProgram>

class GLVideoWidget;

/*!
 * This class is used to swap the front and back frame buffers and do all the
 * drawing on a GLVideoWidget. In order to keep the main thread responsive,
 * GLWorker should be moved to a separate thread with QObject::moveToThread().
 */

class GLWorker : public QObject
{
    Q_OBJECT

public:
    explicit GLWorker(GLVideoWidget *glw);

    void start();
    void stop();

public slots:
    void onDrawFrame(unsigned char* imBuf);
    void onAspectRatioChanged(int w);
    void resizeGL(int w, int h);

signals:
    void vblank();
    void triggerSignal(int);
    void log(const QString&);

private:
    void                    initializeGL();

    GLVideoWidget*          glw_;

    int                     videoWidth_;
    bool                    shouldStop_;
    unsigned char*          buf_;

    QGLShaderProgram        shaderProgram_;
    QVector<QVector2D>      vertices_;
    QVector<QVector2D>      textureCoordinates_;

private slots:
    void startLoop();
    void stopLoop();
};

#endif // GLWORKER_H
