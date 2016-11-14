#pragma once

#include <string>
#include <vector>

#include <QGLWidget>

#include "platform/gl.h"

#include "Sprite.h"

class GLSprite
{
public:
    GLSprite(const QString& string);
    ~GLSprite();
    const std::vector<GLuint>& operator[](quint32 num) const;
    bool Fail() const;

    // int becouse x or y could be negative, don't touch this
    int W() const;
    int H() const;

    Sprite* GetSDLSprite() const;

    unsigned int FrameW() const;
    unsigned int FrameH() const;
private:
    void Init(Sprite* sprite);
    int w_;
    int h_;
    std::vector<std::vector<GLuint>> gl_sprites_;
    Sprite* sdl_sprite_;
    bool fail_;
};
