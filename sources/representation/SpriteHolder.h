#pragma once

#include "Sprite.h"
#include <map>
#include <string>

#include "GLSprite.h"
#include "Screen.h"

class SpriteHolder
{
public:
    SpriteHolder();
    GLSprite* GetSprite(const QString& type);
    void LoadImage(const QString& image);
private:
    std::map<QString, GLSprite*> sprites;
};

SpriteHolder& GetSpriter();
bool IsSpriterValid();
void SetSpriter(SpriteHolder* aspr);
