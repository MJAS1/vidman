/*
 * event.cpp
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

#include <QImage>

#include "event.h"
#include "eventcontainer.h"
#include "eventparser.h"
#include "settings.h"

void Event::apply(cv::Mat &)
{
    if(first_) {
        emit triggered(trigCode_, log_);
        first_ = false;
    }
}

void DelEvent::apply(EventContainer &events)
{
    emit triggered(trigCode_, log_);
    if(tag_ == TYPE)
        events.deleteType(delType_);
    else
        events.deleteId(delId_);

    ready_ = true;
}

FlipEvent::FlipEvent(int start, int axis, int delay, int id, uint8_t trigCode) :
    Event(EVENT_FLIP, start, delay, 0, id, trigCode), axis_(axis)
{
}

void FlipEvent::apply(cv::Mat &frame)
{    
    Event::apply(frame);
    cv::flip(frame, frame, axis_);
}

void FlipEvent::apply(EventContainer &events)
{
    events.deleteType(Event::EVENT_FLIP);
}

FadeInEvent::FadeInEvent(int start, int duration, int delay, int id,
                         uint8_t trigCode) :
    Event(EVENT_FADEIN, start, delay, duration, id, trigCode, FADE_PRIORITY),
    amount_(-255), stopped_(false)
{
    timerWithPause_.invalidate();
}

void FadeInEvent::apply(cv::Mat &frame)
{
    Event::apply(frame);
    if(!stopped_) {
        if(!timerWithPause_.isValid()) {
            timerWithPause_.start();
            interval_ = duration_ / 255;
        }
        int msecsElapsed = timerWithPause_.msecsElapsed();
        amount_ = -255 + msecsElapsed/interval_;
        if(amount_ >= 0) {
            amount_ = 0;
            stopped_ = true;
        }
    }
    frame += cv::Scalar(amount_, amount_, amount_);
}

void FadeInEvent::apply(EventContainer &events)
{
    events.deleteType(Event::EVENT_FADEIN);
    events.deleteType(Event::EVENT_FADEOUT);
}

void FadeInEvent::pause()
{
    stopped_ = true;
    timerWithPause_.pause();
}

void FadeInEvent::unpause()
{
    stopped_ = false;
    timerWithPause_.resume();
}

FadeOutEvent::FadeOutEvent(int start, int duration, int delay, int id,
                           uint8_t trigCode) :
    Event(EVENT_FADEOUT, start, delay, duration, id, trigCode, FADE_PRIORITY),
    amount_(0), stopped_(false)
{
    timerWithPause_.invalidate();
}

void FadeOutEvent::apply(cv::Mat &frame)
{
    Event::apply(frame);
    if(!stopped_) {
        if(!timerWithPause_.isValid()) {
            timerWithPause_.start();
            interval_ = duration_ / 255;
        }
        int msecsElapsed = timerWithPause_.msecsElapsed();
        amount_ = -msecsElapsed/interval_;
        if(amount_ <= -255) {
            amount_ = -255;
            stopped_ = true;
        }
    }
    frame += cv::Scalar(amount_, amount_, amount_);
}

void FadeOutEvent::apply(EventContainer &events)
{
    events.deleteType(Event::EVENT_FADEIN);
    events.deleteType(Event::EVENT_FADEOUT);
}

void FadeOutEvent::pause()
{
    stopped_ = true;
    timerWithPause_.pause();
}

void FadeOutEvent::unpause()
{
    stopped_ = false;
    timerWithPause_.resume();
}

ImageEvent::ImageEvent(int start, const cv::Point2i& pos, const cv::Mat& image,
                       int delay, int id, uint8_t trigCode) :
    Event(EVENT_IMAGE, start, delay, 0, id, trigCode, IMAGE_PRIORITY),
    image_(image), pos_(pos)
{
}

void ImageEvent::apply(cv::Mat &frame)
{
    Event::apply(frame);
    if(!frame.empty() && !image_.empty())
        overlayImage(frame, image_, frame, pos_);
}

/*
 * Code from Jepson's Blog:
 * http://jepsonsblog.blogspot.fi/2012/10/overlay-transparent-image-in-opencv.html
 */
void ImageEvent::overlayImage(const cv::Mat &background, const cv::Mat &foreground,
  cv::Mat &output, const cv::Point2i& location)
{
  background.copyTo(output);

  // start at the row indicated by location, or at row 0 if location.y is negative.
  for(int y = std::max(location.y , 0); y < background.rows; ++y) {
    int fY = y - location.y; // because of the translation

    // we are done of we have processed all rows of the foreground image.
    if(fY >= foreground.rows)
      break;

    // start at the column indicated by location,

    // or at column 0 if location.x is negative.
    for(int x = std::max(location.x, 0); x < background.cols; ++x)
    {
      int fX = x - location.x; // because of the translation.

      // we are done with this row if the column is outside of the foreground image.
      if(fX >= foreground.cols)
        break;

      // determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
      double opacity =
        ((double)foreground.data[fY * foreground.step + fX * foreground.channels() + 3])/ 255.;


      // and now combine the background and foreground pixel, using the opacity,

      // but only if opacity > 0.
      for(int c = 0; opacity > 0 && c < output.channels(); ++c)
      {
        unsigned char foregroundPx =
          foreground.data[fY * foreground.step + fX * foreground.channels() + c];
        unsigned char backgroundPx =
          background.data[y * background.step + x * background.channels() + c];
        output.data[y*output.step + output.channels()*x + c] =
          backgroundPx * (1.-opacity) + foregroundPx * opacity;
      }
    }
  }
}

TextEvent::TextEvent(int start, const QString& str, cv::Scalar color,
                     const cv::Point2i& pos, int delay, int id, uint8_t trigCode) :
    Event(EVENT_TEXT, start, delay, 0, id, trigCode), color_(color),
    pos_(pos), str_(str)
{
}

void TextEvent::apply(cv::Mat &frame)
{
    Event::apply(frame);
    cv::putText(frame, str_.toStdString(), pos_, cv::FONT_HERSHEY_DUPLEX, 1,
                color_, 2);
}

RotateEvent::RotateEvent(int start, int angle, int delay, int id, uint8_t trigCode)
    : Event(EVENT_ROTATE, start, delay, 0, id, trigCode), angle_(angle)
{
}

void RotateEvent::apply(cv::Mat &frame)
{
    Event::apply(frame);
    cv::Point2f center(frame.cols/2., frame.rows/2.);
    cv::Mat rotMat = getRotationMatrix2D(center, angle_, 1.0);
    if(!frame.empty())
        cv::warpAffine(frame, frame, rotMat, cv::Size(frame.cols, frame.rows+1));
}

void RotateEvent::apply(EventContainer &events)
{
    events.deleteType(Event::EVENT_ROTATE);
}

FreezeEvent::FreezeEvent(int start, int delay, int id, uint8_t trigCode)
        : Event(EVENT_FREEZE, start, delay, 0, id, trigCode, FREEZE_PRIORITY),
          started_(false)
{
}

void FreezeEvent::apply(cv::Mat &frame)
{
    Event::apply(frame);
    if(!started_) {
        frame.copyTo(frame_);
        started_ = true;
    }
    frame_.copyTo(frame);
}

void FreezeEvent::apply(EventContainer &events)
{
    events.deleteType(Event::EVENT_FREEZE);
}

ZoomEvent::ZoomEvent(int start, double scale, int duration, int delay, int id,
                     uint8_t trigCode) :
        Event(EVENT_ZOOM, start, delay, duration, id, trigCode), scale_(scale)
{
}

void ZoomEvent::apply(cv::Mat &frame)
{
    Event::apply(frame);
    cv::Mat tmp;
    resize(frame, tmp, cv::Size(), scale_, scale_, cv::INTER_LINEAR);

    // Take only the center 640x480 of the resized image
    cv::Point p((tmp.cols - frame.cols) / 2, (tmp.rows - frame.rows) / 2);
    cv::Rect roi(p, frame.size());
    frame = tmp(roi).clone();
}

RecordEvent::RecordEvent(int start, VideoPtr video, int delay, int duration,
                         int id, uint8_t trigCode) :
    Event(EVENT_RECORD, start, delay, duration, id, trigCode, RECORD_PRIORITY),
    video_(video), finished_(false), paused_(false)
{
    timer_.invalidate();
}

void RecordEvent::apply(cv::Mat &frame)
{
    Event::apply(frame);
    if(!finished_ && !paused_) {
        if(!timer_.isValid())
            timer_.start();

        if(timer_.msecsElapsed() < duration_)
            video_->frames_.append(Frame{frame.clone(), 0});
        else
            finished_ = true;
    }

}

void RecordEvent::pause()
{
    paused_ = true;
    timer_.pause();
}

void RecordEvent::unpause()
{
    paused_ = false;
    timer_.resume();
}

PlaybackEvent::PlaybackEvent(int start, VideoPtr video, int delay, int duration,
                             int id, uint8_t trigCode) :
    Event(EVENT_PLAYBACK, start, delay, duration, id, trigCode,
          PLAYBACK_PRIORITY),
    video_(video), finished_(false), paused_(false)
{
    iter_ = video_->frames_.begin();
}

void PlaybackEvent::apply(cv::Mat &frame)
{
    emit triggered(iter_->trigCode_, "Video playback.");
    Event::apply(frame);
    if(video_->frames_.empty()) {
        ready_ = true;
    }
    if(!paused_) {
        iter_->data_.copyTo(frame);
        if(++iter_ == video_->frames_.end())
            ready_ = true;
    }
}

void PlaybackEvent::pause()
{
    paused_ = true;
}

void PlaybackEvent::unpause()
{
    paused_ = false;
}

MotionDetectorEvent::MotionDetectorEvent(int start, int target, int tolerance,
                                         int delay, int id, uint8_t trigCode,
                                         uint8_t successCode, uint8_t failCode,
                                         int x, int y, State state) :
            Event(EVENT_DETECT_MOTION, start, delay, 0, id, trigCode,
            MOTION_DETECTOR_PRIORITY), state_(state), successCode_(successCode),
            failCode_(failCode), target_(target), tolerance_(tolerance)
{
    Settings settings;
    threshold_ = settings.movementThreshold;
    textPos_ = cv::Point(x, y);

    /*
     * Detect movement only from this area. ROI might have to be changed/removed
     * for future experiments.
     */
    roi_ = cv::Rect(0, 125, 640, 225);
}

MotionDetectorEvent::MotionDetectorEvent() :
    Event(EVENT_DETECT_MOTION), state_(MOTION_DIALOG)
{
    roi_ = cv::Rect(0, 125, 640, 225);
}

void MotionDetectorEvent::apply(cv::Mat &frame)
{
    if(first_) {
        movementTimer_.start();
        first_ = false;
    }

    if(next_.empty()) {
        next_ = frame.clone()(roi_);
        current_ = next_;
        prev_ = next_;
    }
    else {
        prev_ = current_;
        current_ = next_;
        next_ = frame.clone()(roi_);
    }

    switch(state_) {
        // Wating for the movement to start
        case WAITING:
            waiting();
            break;
        //Movement has started
        case TRACKING:
            tracking();
            break;
        // Movement has maybe finished. Make sure that it really finished instead
        // of a brief pause due to, e.g., a change in direction.
        case MAYBE_FINISHED:
            maybeFinished();
            break;
        // Movement finished, draw duration feedback.
        case FINISHED:
            finished(frame);
            break;
        // This state is only used for an event associated with a motion dialog.
        case MOTION_DIALOG:
            nChanges();
            createMotionPixmap();
            break;
    }
}

void MotionDetectorEvent::waiting()
{
    if(nChanges() > threshold_) {
        emit triggered(trigCode_, log_);
        state_ = TRACKING;
    }
}

void MotionDetectorEvent::tracking()
{
    if(nChanges() < threshold_) {
        // Timer to keep track of how long the image has been still.
        finishTimer_.start();
        state_ = MAYBE_FINISHED;
    }
}

void MotionDetectorEvent::maybeFinished()
{
    if(nChanges() < threshold_) {
        if(finishTimer_.elapsed() > MinStopTime) {
            time_ = movementTimer_.elapsed();
            state_ = FINISHED;

            //Movement finished within time limit.
            if(time_ < target_ + tolerance_ && time_ > target_ - tolerance_) {
                emit triggered(successCode_, QString("Movement finished."));
                cv::Scalar green(0, 255, 0);
                color_ = green;
            }
            //Movement was too fast or too slow
            else {
                emit triggered(failCode_, QString("Movement finished."));
                cv::Scalar red(0, 0, 255);
                color_ = red;
            }
            finishTimer_.restart();

            /*
             * After the movement has finished, this event doesn't track
             * movement anymore and is just used to write the duration of the
             * movement on the frame. Thus, the priority needs to be changed.
             * The priorityChanged() signal is connected to the sort() slot of
             * eventcontainer so that it is resorted by priority.
             */
            priority_ = DEFAULT_PRIORITY;
            emit priorityChanged();
        }
    }
    else
        state_ = TRACKING;
}

void MotionDetectorEvent::finished(cv::Mat &frame)
{
    // Draw the duration of the movement on the frame.
    if(finishTimer_.elapsed() < TextDuration)
        cv::putText(frame, std::string(std::to_string(time_)),
                    textPos_, cv::FONT_HERSHEY_DUPLEX, 1, color_, 2);
    else
        ready_=true;
}

int MotionDetectorEvent::nChanges()
{
    const int White = 255;

    /*
     * Create a black and white differential image, where white pixels represent
     * a difference.
     */
    cv::Mat d1, d2;
    cv::absdiff(prev_, next_, d1);
    cv::absdiff(current_, next_, d2);
    cv::bitwise_and(d1, d2, result_);
    cv::cvtColor(result_, result_, cv::COLOR_BGR2GRAY);
    cv::threshold(result_, result_, 5, White, cv::THRESH_BINARY);
    cv::erode(result_,result_,cv::Mat());
    cv::dilate(result_,result_,cv::Mat());

    int changes = 0;
    int min_x = result_.cols, min_y = result_.rows;
    int max_x = 0, max_y = 0;

    // Compute the amount of white pixels.
    for(int i = 0; i < roi_.width; i += 2) {
        for(int j = 0; j < roi_.height; j += 2) {
            if(static_cast<int>(result_.at<uchar>(j, i)) == White) {
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

void MotionDetectorEvent::apply(EventContainer &events)
{
    events.deleteType(Event::EVENT_DETECT_MOTION);
    connect(this, SIGNAL(priorityChanged()), &events, SLOT(sort()),
            Qt::QueuedConnection);
}

void MotionDetectorEvent::createMotionPixmap()
{
    QImage img;
    img = QImage(result_.data, result_.cols, result_.rows, result_.step,
                 QImage::Format_Indexed8);
    QPixmap pixmap;
    pixmap.convertFromImage(img.rgbSwapped());
    emit pixmapReady(pixmap);
}

PauseEvent::PauseEvent()
{
    int baseline = 0;
    cv::Size size1 = getTextSize(std::string("Press space to continue"),
                                 cv::FONT_HERSHEY_DUPLEX, 1, 1, &baseline);
    cv::Size size2 = getTextSize(std::string("Paused"), cv::FONT_HERSHEY_DUPLEX,
                                 1, 1, &baseline);
    txtPos1_ = cv::Point((VIDEO_WIDTH - size2.width)/2,
                       (VIDEO_HEIGHT + size2.height)/2);
    txtPos2_ = cv::Point((VIDEO_WIDTH - size1.width)/2,
                       (VIDEO_HEIGHT + size1.height)/2 + size2.height + 10);
}

void PauseEvent::apply(cv::Mat &frame)
{
    Event::apply(frame);
    cv::putText(frame, std::string("Paused"), txtPos1_, cv::FONT_HERSHEY_DUPLEX,
                1, cv::Scalar(255,255,255));
    cv::putText(frame, std::string("Press space to continue"), txtPos2_,
                cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255,255,255));
}

void PauseEvent::unpause()
{
    ready_ = true;
}
