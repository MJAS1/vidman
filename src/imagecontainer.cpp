#include "imagecontainer.h"

ImageContainer::ImageContainer()
{
}

ImageContainer::~ImageContainer()
{
    for(Iterator iter = begin(); iter != end(); iter++)
        delete iter.value();
}

bool ImageContainer::addImage(const int id, const QString &filename)
{
    Iterator iter = find(id);
    if(iter != end()) {
        delete iter.value();
        erase(iter);
    }

    cv::Mat file = cv::imread(filename.toStdString(), CV_LOAD_IMAGE_UNCHANGED);
    if(file.empty())
        return false;

    cv::Mat *image = new cv::Mat(file);
    QMap<int, cv::Mat*>::operator [](id) = image;

    return true;
}

const cv::Mat& ImageContainer::operator [](int id) const
{
    ConstIterator iter = find(id);
    return *(iter.value());
}
