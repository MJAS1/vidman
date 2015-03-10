#ifndef IMAGECONTAINER_H
#define IMAGECONTAINER_H

#include <QMap>
#include <QString>
#include <opencv/cv.h>
#include <opencv/highgui.h>


class ImageContainer : public QMap<int, cv::Mat*>
{
public:
    ImageContainer();
    ~ImageContainer();

    bool addImage(const int id, const QString& filename);
    const cv::Mat& operator[](int id) const;

private:
    ImageContainer(const ImageContainer&);
    ImageContainer& operator =(const ImageContainer);
};

#endif // IMAGECONTAINER_H
