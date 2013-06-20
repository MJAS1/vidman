#include "event.h"

Event::Event(EventType type, float start, float delay, int id) : type(type), start(start), delay(delay), id(id)
{
}

Event::~Event()
{

}

float Event::getStart() const
{
    return start;
}


float Event::getDelay() const
{
    return delay;
}

EventType Event::getType() const
{
    return type;
}

int Event::getId() const
{
    return id;
}

int RemoveEvent::apply(cv::Mat &frame)
{
    return getId();
}

int FlipEvent::apply(cv::Mat &frame)
{
    cv::flip(frame, frame, 1);
    return -1;
}

FadeInEvent::FadeInEvent(float start, float duration, float delay, int id) :
    Event(EVENT_FADEIN, start, delay, id), amount(-255), duration(duration), stopped(false)
{
    timer = new QTimer;
    connect(timer, SIGNAL(timeout()), this, SLOT(increaseAmount()));
}

void FadeInEvent::increaseAmount()
{
    amount++;
    if(amount >= 0)
    {
        timer->stop();
        stopped = true;
    }
}

int FadeInEvent::apply(cv::Mat &frame)
{
    if(!timer->isActive() && !stopped)
        timer->start(1000.0/255.0*duration);

    frame += cv::Scalar(amount, amount, amount);

    return -1;
}

FadeOutEvent::FadeOutEvent(float start, float duration, float delay, int id) :
    Event(EVENT_FADEOUT, start, delay, id), amount(0), duration(duration), stopped(false)
{
    timer = new QTimer;
    connect(timer, SIGNAL(timeout()), this, SLOT(decreaseAmount()));
}

void FadeOutEvent::decreaseAmount()
{
    amount--;
    if(amount <= -255)
    {
        timer->stop();
        stopped = true;
    }
}


int FadeOutEvent::apply(cv::Mat &frame)
{
    if(!timer->isActive() && !stopped)
        timer->start(1000.0/255.0*duration);

    frame += cv::Scalar(amount, amount, amount);

    return -1;
}

ImageEvent::ImageEvent(float start, cv::Point2i pos, const cv::Mat& image, float delay, int id) :
    Event(EVENT_IMAGE, start, delay, id), image(image), pos(pos)
{
}

int ImageEvent::apply(cv::Mat &frame)
{
    if(!frame.empty() && !image.empty())
        overlayImage(frame, image, frame, pos);

    return -1;
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

int TextEvent::apply(cv::Mat &frame)
{
    cv::putText(frame, str.toStdString(), pos, cv::FONT_HERSHEY_DUPLEX, 2, cv::Scalar(256, 100, 100), 2);
    return -1;
}

int RotateEvent::apply(cv::Mat &frame)
{
    cv::Point2f center(frame.cols/2., frame.rows/2.);
    cv::Mat rotMat = getRotationMatrix2D(center, angle, 1.0);
    cv::warpAffine(frame, frame, rotMat, cv::Size(frame.cols, frame.rows+1));

    return -1;
}

int FreezeEvent::apply(cv::Mat &frame)
{
    if(!started)
    {
        frame.copyTo(freezedFrame);
        started = true;
    }
    freezedFrame.copyTo(frame);
    return -1;
}
