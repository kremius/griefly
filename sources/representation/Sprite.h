#pragma once

#include <QImage>

#include "Metadata.h"

#include "core/Constheader.h"

struct InitSprite
{
    QString file;
    int numFrameH;
    int numFrameW;
};

class Sprite
{
public:
    Sprite();
    ImageMetadata metadata;
    int h;
    int w;
    int frames_h_;
    int frames_w_;
    std::vector<QImage> frames;
    bool init(InitSprite data);
    ~Sprite();
};
