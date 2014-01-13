#include "event.h"

Event::Event(EventType type, int start, int delay, int duration, int id, int trigCode) :
    type(type), start(start), delay(delay), duration(duration), id(id), trigCode(trigCode)
{
}

Event::~Event()
{

}

int Event::getStart() const
{
    return start;
}


int Event::getDelay() const
{
    return delay;
}

int Event::getDuration() const
{
    return duration;
}

EventType Event::getType() const
{
    return type;
}

int Event::getId() const
{
    return id;
}

void Event::appendLog(const QString &str)
{
    log = str;
}

QString Event::getLog() const
{
    return log;
}

int Event::getTrigCode() const
{
    return trigCode;
}

void Event::pause()
{
}

void Event::unpause()
{
}

RemoveEvent::RemoveEvent(int start, int delay, int removeId, int trigCode) :
    Event(EVENT_REMOVE, start, delay, 0, -1, trigCode), removeId(removeId), removeType(EVENT_NULL)
{
}

RemoveEvent::RemoveEvent(int start, int delay, EventType removeType, int trigCode) :
    Event(EVENT_REMOVE, start, delay, 0, -1, trigCode), removeType(removeType)
{
}

void RemoveEvent::apply(cv::Mat &frame)
{
}

EventType RemoveEvent::getRemoveType() const
{
    return removeType;
}

int RemoveEvent::getRemoveId() const
{
    return removeId;
}

FlipEvent::FlipEvent(int start, int delay, int id, int trigCode) :
    Event(EVENT_FLIP, start, delay, 0, id, trigCode)
{
}

void FlipEvent::apply(cv::Mat &frame)
{
    cv::flip(frame, frame, 1);
}

FadeInEvent::FadeInEvent(int start, int duration, int delay, int id, int trigCode) :
    Event(EVENT_FADEIN, start, delay, duration, id, trigCode), amount(-255), stopped(false)
{
    timerWithPause.invalidate();
}

void FadeInEvent::apply(cv::Mat &frame)
{
    if(!stopped)
    {
        if(!timerWithPause.isValid())
        {
            timerWithPause.start();
            interval = duration/255;
        }
        int msecsElapsed = timerWithPause.nsecsElapsed()/1000000;
        amount = -255 + msecsElapsed/interval;
        if(amount >= 0)
        {
            amount = 0;
            stopped = true;
        }
    }
    frame += cv::Scalar(amount, amount, amount);
}

void FadeInEvent::pause()
{
    stopped = true;
    timerWithPause.pause();
}

void FadeInEvent::unpause()
{
    stopped = false;
    timerWithPause.resume();
}

FadeOutEvent::FadeOutEvent(int start, int duration, int delay, int id, int trigCode) :
    Event(EVENT_FADEOUT, start, delay, duration, id, trigCode), amount(0), stopped(false)
{
    timerWithPause.invalidate();
}

void FadeOutEvent::apply(cv::Mat &frame)
{
    if(!stopped)
    {
        if(!timerWithPause.isValid())
        {
            timerWithPause.start();
            interval = duration/255;
        }
        int msecsElapsed = timerWithPause.nsecsElapsed()/1000000;
        amount = -msecsElapsed/interval;
        if(amount <= -255)
        {
            amount = -255;
            stopped = true;
        }
    }
    frame += cv::Scalar(amount, amount, amount);
}

void FadeOutEvent::pause()
{
    stopped = true;
    timerWithPause.pause();
}

void FadeOutEvent::unpause()
{
    stopped = false;
    timerWithPause.resume();
}

ImageEvent::ImageEvent(int start, cv::Point2i pos, const cv::Mat& image, int delay, int id, int trigCode) :
    Event(EVENT_IMAGE, start, delay, 0, id, trigCode), image(image), pos(pos)
{
}

void ImageEvent::apply(cv::Mat &frame)
{
    if(!frame.empty() && !image.empty())
        overlayImage(frame, image, frame, pos);
}

//Code from Jepson's Blog http://jepsonsblog.blogspot.fi/2012/10/overlay-transparent-image-in-opencv.html
void ImageEvent::overlayImage(const cv::Mat &background, const cv::Mat &foreground,
  cv::Mat &output, cv::Point2i location)
{
  background.copyTo(output);


  // start at the row indicated by location, or at row 0 if location.y is negative.
  for(int y = std::max(location.y , 0); y < background.rows; ++y)
  {
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

TextEvent::TextEvent(int start, QString str, cv::Scalar color, cv::Point2i pos, int delay, int id, int trigCode) :
    Event(EVENT_TEXT, start, delay, 0, id, trigCode), color(color), pos(pos), str(str)
{
}

void TextEvent::apply(cv::Mat &frame)
{
    cv::putText(frame, str.toStdString(), pos, cv::FONT_HERSHEY_DUPLEX, 1, color, 2);
}

RotateEvent::RotateEvent(int start, int angle, int delay, int id, int trigCode)
    : Event(EVENT_ROTATE, start, delay, 0, id, trigCode), angle(angle)
{
}

void RotateEvent::apply(cv::Mat &frame)
{
    cv::Point2f center(frame.cols/2., frame.rows/2.);
    cv::Mat rotMat = getRotationMatrix2D(center, angle, 1.0);
    cv::warpAffine(frame, frame, rotMat, cv::Size(frame.cols, frame.rows+1));
}

FreezeEvent::FreezeEvent(int start, int delay, int id, int trigCode)
                    : Event(EVENT_FREEZE, start, delay, 0, id, trigCode), started(false)
{
}

void FreezeEvent::apply(cv::Mat &frame)
{
    if(!started)
    {
        frame.copyTo(freezedFrame);
        started = true;
    }
    freezedFrame.copyTo(frame);
}

DetectMotionEvent::DetectMotionEvent(int start, int delay, int trigCode) : Event(EVENT_DETECT_MOTION, start, delay, 0, -1, trigCode)
{
}

void DetectMotionEvent::apply(cv::Mat &frame)
{
}
