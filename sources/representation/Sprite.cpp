#include "Sprite.h"

#include <QGLWidget>
#include <QDebug>

bool Sprite::init(InitSprite data)
{
    QImage image;
    if (!image.load(data.file))
    {
        qDebug() << "Image load error: " << data.file;
        return false;
    }

    metadata.Init(data.file, image.width(), image.height());
    if (metadata.Valid())
    {
        frames_w_ = image.width() / metadata.GetW();
        frames_h_ = image.height() / metadata.GetH();
    }
    else
    {
        frames_h_ = data.numFrameH;
        frames_w_ = data.numFrameW;
    }
    if((frames_h_ == 0) || (frames_w_ == 0))
    {        
        w = image.width();
        h = image.height();
        frames_w_ = 1;
        frames_h_ = 1;
    }
    else
    {
        w = image.width() / frames_w_;
        h = image.height() / frames_h_;
    }
    qDebug() << frames_w_ << "x" << frames_h_ << " - loaded " << data.file;

    int x = 0;
    int y = 0;
    frames.resize(frames_w_ * frames_h_);

    for(int j = 0; j < frames_h_; ++j)
    {
        y = j * h;
        for(int i = 0; i < frames_w_; ++i)
        {
            x = i * w;
            // frames[i * numFrameH + j] = QGLWidget::convertToGLFormat(image.copy(x, y, w, h));
            frames[i * frames_h_ + j] = image.copy(x, y, w, h);
        }
    }
    return true;
}

Sprite::Sprite()
{
    h = 0;
    w = 0;
    frames_h_ = 0;
    frames_w_ = 0;
}

Sprite::~Sprite()
{

}
