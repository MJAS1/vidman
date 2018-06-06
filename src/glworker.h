/*
 * glworker.h
 *
 * Author: Manu Sutela
 * Copyright (C) 2018 Aalto University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GLWORKER_H
#define GLWORKER_H

#include <QObject>
#include <QGLShaderProgram>
#include "cycdatabuffer.h"

class GLVideoWidget;

/*!
 * This class is used to swap the front and back frame buffers and do all the
 * drawing on a GLVideoWidget. In order to keep the main thread responsive,
 * GLWorker should be moved to a separate thread with QObject::moveToThread().
 *
 * The class emits the signal vblank(const ChunkAttrib*) each time the buffers
 * have been swapped so that other functionality can be synced with the vertical
 * refresh rate.
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
    void vblank(const ChunkAttrib*);

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
