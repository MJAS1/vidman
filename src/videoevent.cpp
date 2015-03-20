#include <QDebug>
#include "videoevent.h"

VideoEvent::VideoEvent(EventType type, int start, int delay, int duration, int id, int trigCode, int priority) :
    Event(type,start, delay, duration, id, trigCode), priority_(priority)
{
}

VideoEvent::~VideoEvent()
{
}


FlipEvent::FlipEvent(int start, int delay, int id, int trigCode) :
    VideoEvent(EVENT_FLIP, start, delay, 0, id, trigCode)
{
}

void FlipEvent::apply(cv::Mat &frame)
{
    cv::flip(frame, frame, 1);
}

FadeInEvent::FadeInEvent(int start, int duration, int delay, int id, int trigCode) :
    VideoEvent(EVENT_FADEIN, start, delay, duration, id, trigCode), amount_(-255), stopped_(false)
{
    timerWithPause_.invalidate();
}

void FadeInEvent::apply(cv::Mat &frame)
{
    if(!stopped_) {
        if(!timerWithPause_.isValid()) {
            timerWithPause_.start();
            interval_ = duration_/255;
        }
        int msecsElapsed = timerWithPause_.nsecsElapsed()/1000000;
        amount_ = -255 + msecsElapsed/interval_;
        if(amount_ >= 0) {
            amount_ = 0;
            stopped_ = true;
        }
    }
    frame += cv::Scalar(amount_, amount_, amount_);
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

FadeOutEvent::FadeOutEvent(int start, int duration, int delay, int id, int trigCode) :
    VideoEvent(EVENT_FADEOUT, start, delay, duration, id, trigCode), amount_(0), stopped_(false)
{
    timerWithPause_.invalidate();
}

void FadeOutEvent::apply(cv::Mat &frame)
{
    if(!stopped_) {
        if(!timerWithPause_.isValid()) {
            timerWithPause_.start();
            interval_ = duration_/255;
        }
        int msecsElapsed = timerWithPause_.nsecsElapsed()/1000000;
        amount_ = -msecsElapsed/interval_;
        if(amount_ <= -255) {
            amount_ = -255;
            stopped_ = true;
        }
    }
    frame += cv::Scalar(amount_, amount_, amount_);
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

ImageEvent::ImageEvent(int start, const cv::Point2i& pos, const cv::Mat& image, int delay, int id, int trigCode) :
    VideoEvent(EVENT_IMAGE, start, delay, 0, id, trigCode), image_(image), pos_(pos)
{
}

void ImageEvent::apply(cv::Mat &frame)
{
    if(!frame.empty() && !image_.empty())
        overlayImage(frame, image_, frame, pos_);
}

//Code from Jepson's Blog http://jepsonsblog.blogspot.fi/2012/10/overlay-transparent-image-in-opencv.html
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

TextEvent::TextEvent(int start, const QString& str, cv::Scalar color, const cv::Point2i& pos, int delay, int id, int trigCode) :
    VideoEvent(EVENT_TEXT, start, delay, 0, id, trigCode), color_(color), pos_(pos), str_(str)
{
}

void TextEvent::apply(cv::Mat &frame)
{
    cv::putText(frame, str_.toStdString(), pos_, cv::FONT_HERSHEY_DUPLEX, 1, color_, 2);
}

RotateEvent::RotateEvent(int start, int angle, int delay, int id, int trigCode)
    : VideoEvent(EVENT_ROTATE, start, delay, 0, id, trigCode), angle_(angle)
{
}

void RotateEvent::apply(cv::Mat &frame)
{
    cv::Point2f center(frame.cols/2., frame.rows/2.);
    cv::Mat rotMat = getRotationMatrix2D(center, angle_, 1.0);
    cv::warpAffine(frame, frame, rotMat, cv::Size(frame.cols, frame.rows+1));
}

FreezeEvent::FreezeEvent(int start, int delay, int id, int trigCode, int priority)
        : VideoEvent(EVENT_FREEZE, start, delay, 0, id, trigCode, priority), started_(false)
{
}

void FreezeEvent::apply(cv::Mat &frame)
{
    if(!started_) {
        frame.copyTo(freezedFrame_);
        started_ = true;
    }
    freezedFrame_.copyTo(frame);
}

ZoomEvent::ZoomEvent(int start, double scale, int duration, int delay, int id, int trigCode) :
        VideoEvent(EVENT_ZOOM, start, delay, duration, id, trigCode),
        scale_(scale), stopped_(false)
{
    timer_.invalidate();
}

void ZoomEvent::apply(cv::Mat &frame)
{
    if(!stopped_) {
        if(!timer_.isValid()) {
            interval_ = (scale_ - 1) / duration_;
            timer_.start();
        }
        int msecsElapsed = timer_.nsecsElapsed()/1000000;
        coef_ = 1 + interval_*msecsElapsed;
        if(coef_ >= scale_) {
            coef_ = scale_;
            stopped_ = true;
        }
    }

    cv::Mat tmp;
    cv::Size newSize(frame.cols * coef_, frame.rows * coef_);
    resize(frame, tmp, newSize);

    cv::Point p((tmp.cols - frame.cols) / 2, (tmp.rows - frame.rows) / 2);
    cv::Rect roi(p, frame.size());
    frame = tmp(roi).clone();
}

void ZoomEvent::pause()
{
    stopped_ = true;
    timer_.pause();
}

void ZoomEvent::unpause()
{
    stopped_ = false;
    timer_.resume();
}

RecordEvent::RecordEvent(int start, FramesPtr frames, int delay, int duration, int id, int trigCode, int priority) :
    VideoEvent(EVENT_RECORD, start, delay, duration, id, trigCode, priority), frames_(frames), finished_(false), paused_(false)
{
    timer_.invalidate();
}

void RecordEvent::apply(cv::Mat &frame)
{
    if(!finished_ && !paused_) {
        if(!timer_.isValid())
            timer_.start();

        if(timer_.nsecsElapsed()/1000000 < duration_)
            frames_->append(frame.clone());
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

PlaybackEvent::PlaybackEvent(int start, FramesPtr frames, int delay, int duration, int id, int trigCode, int priority) :
    VideoEvent(EVENT_RECORD, start, delay, duration, id, trigCode, priority), frames_(frames), finished_(false), paused_(false)
{
    iter_ = frames_->begin();
}

void PlaybackEvent::apply(cv::Mat &frame)
{
    if(!finished_ && !paused_) {
        iter_->copyTo(frame);
        if(++iter_ == frames_->end())
            finished_ = true;
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
