/*
 * cameraworker.h
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

#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>
#include <opencv2/opencv.hpp>

#include "eventcontainer.h"
#include "event.h"

class CycDataBuffer;
class Camera;

/*!
 * This class acquires, processes and timestamps frames for a single Camera
 * object. The processed frames are subsequently inserted to the cyclic data
 * buffer. Events in the defaultEvents_ container are applied to each frame by
 * default. Other events are added with the addEvent()-function.
 *
 * Instances of this class should be moved to the same QThread with the glWorker
 * to ensure that they are correctly synced together.
 */
class CameraWorker : public QObject
{
    Q_OBJECT
public:
    explicit CameraWorker(CycDataBuffer* cycBuf, Camera& cam);

    // Adds a new video event to be applied for each frame.
    void addEvent(EventPtr ev);
    void clearEvents();
    void pause();
    void unpause();

signals:
    // Emits a "motion" QPixmap that can be drawn, e.g., to the motionDialog.
    void motionPixmapReady(const QPixmap&);

public slots:
    /*
     * If the motionDialog is shown, a motion detector event is added to
     * defaultEvents_. When the dialog is hidden, the event is deleted.
     */
    void motionDialogToggled(bool);

    // Grabs a new frame, processes it and inserts it to cycBuf_.
    void captureFrame();

private slots:
    // This slot is triggered by event onset.
    void onEventTriggered(uint8_t trigCode, const QString&);

private:
    CycDataBuffer*  cycBuf_;
    EventContainer* events_;
    EventContainer* defaultEvents_;
    Camera&         cam_;

    QMutex          mutex_;
    QString         log_;
    cv::Mat         frame_;

    uint8_t         trigCode_;

    /*
     * These trigger codes are used by default for each new frame, and they can,
     * thus, be used to detect frame changes on MEG data.
     */
    uint8_t         defaultTrig1_;
    uint8_t         defaultTrig2_;
};

#endif // CAMERAWORKER_H
