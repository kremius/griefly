#include "SpriteHolder.h"

GLSprite* SpriteHolder::GetSprite(const QString& type)
{
    if(sprites.find(type) == sprites.end())
        sprites[type] = new GLSprite(type);
    return sprites[type];
}

SpriteHolder::SpriteHolder()
{
};

SpriteHolder* aspr_ = nullptr;
SpriteHolder& GetSpriter()
{
    return *aspr_;
}

bool IsSpriterValid()
{
    return aspr_ != nullptr;
}

void SetSpriter(SpriteHolder* aspr)
{
    aspr_ = aspr;
}

void SpriteHolder::LoadImage(const QString& image)
{
    sprites[image] = new GLSprite(image);
}
