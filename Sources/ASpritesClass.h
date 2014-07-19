#pragma once

#include "sprite.h"
#include <map>
#include <string>

#include "GLSprite.h"
#include "Screen.h"

class ASprClass
{
public:
    ASprClass();
    GLSprite* returnSpr(const std::string& type);//use it for ptr to sprite(try load sprite if isnt exist)
    TTF_Font* font;
    void LoadImage(const std::string& image);
private:
    std::map<std::string, GLSprite*> sprites;
};

ASprClass* GetSpriter();
void SetSpriter(ASprClass* aspr);