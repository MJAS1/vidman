#include <iostream>
#include "settings.h"
#include "common.h"
#include "motiondetector.h"

MotionDetector::MotionDetector() : isTracking_(false), color_(true), event_(nullptr)
{
    Settings settings;
    if(settings.flip)
        events_.append(EventPtr(new FlipEvent(0, 0)));

    if(settings.turnAround)
        events_.append(EventPtr(new RotateEvent(0, 180, 0)));

    sensitivity_ = settings.movementSensitivity;
}

bool MotionDetector::movementDetected(const cv::Mat &frame)
{
    if(next_.empty()) {
        next_ = frame.clone();
        current_ = next_;
        prev_ = next_;
    }
    else {
        prev_ = current_;
        current_ = next_;
        next_ = frame.clone();
    }

    int changes = nChanges();

    movement_ = next_.clone();
    if(changes > sensitivity_) {
        if(min_x-10 > 0) min_x -= 10;
        if(min_y-10 > 0) min_y -= 10;
        if(max_x+10 < result_.cols-1) max_x += 10;
        if(max_y+10 < result_.rows-1) max_y += 10;

        // draw rectangle round the changed pixel
        cv::Point x(min_x,min_y);
        cv::Point y(max_x,max_y);
        cv::Rect rect(x,y);
        cv::rectangle(movement_,rect,cv::Scalar(0, 0, 255),2);
        if(isTracking_) {
            movementPixmap();
            isTracking_ = false;
            return true;
        }
    }

    movementPixmap();
    return false;
}

int MotionDetector::nChanges()
{
    cv::Mat d1, d2;
    cv::absdiff(prev_, next_, d1);
    cv::absdiff(current_, next_, d2);
    cv::bitwise_and(d1, d2, result_);
    cv::cvtColor(result_, result_, CV_BGR2GRAY);
    cv::threshold(result_, result_, 5, 255, CV_THRESH_BINARY);
    cv::erode(result_,result_,cv::Mat());
    cv::dilate(result_,result_,cv::Mat());

    int changes = 0;
    min_x = result_.cols; min_y = result_.rows;
    max_x = 0;
    max_y = 0;

    for(int i = 0; i < VIDEO_WIDTH; i++) {
        for(int j = 0; j < VIDEO_HEIGHT; j++) {
            if(static_cast<int>(result_.at<uchar>(j, i)) == 255) {
                changes++;
                if(min_x>i) min_x = i;
                if(max_x<i) max_x = i;
                if(min_y>j) min_y = j;
                if(max_y<j) max_y = j;
            }
        }
    }

    return changes;
}

void MotionDetector::movementPixmap()
{
    QImage img;
    if(color_) {
        events_.applyEvents(movement_);
        img = QImage(movement_.data, movement_.cols, movement_.rows, movement_.step, QImage::Format_RGB888);
    }
    else {
        events_.applyEvents(result_);
        img = QImage(result_.data, result_.cols, result_.rows, result_.step, QImage::Format_Indexed8);
    }
    QPixmap pixmap;
    pixmap.convertFromImage(img.rgbSwapped());
    emit pixmapReady(pixmap);
}

void MotionDetector::startTracking(EventPtr ev)
{
    event_ = std::move(ev);
    isTracking_ = true;
}

int MotionDetector::getEventTrigCode() const
{
    if(event_)
        return event_->getTrigCode();

    return 0;
}

QString MotionDetector::getEventLog() const
{
    if(event_)
        return event_->getLog();

    return QString();
}

void MotionDetector::changeMovementFrameColor(bool color)
{
    color_ = color;
}
