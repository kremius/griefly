#pragma once

#include "GLSprite.h"

class Screen
{
public:
    Screen(unsigned int x, unsigned int y, bool fullscreen = false);
    void Draw(const GLSprite* sprite, int x, int y, int imageW, int imageH, float angle = 0.0f);
    void ResetScreen(int x, int y);
    void Clear();
    void Swap();
    bool Fail();

    void PerformSizeUpdate();

    void NormalizePixels(int* x, int* y)
    {
        *x = static_cast<int>
            (  static_cast<float>(*x)
            * (  static_cast<float>(sizeW)
               / static_cast<float>(w())
               ));
        *y = static_cast<int>
            (  static_cast<float>(*y)
            * (  static_cast<float>(sizeH)
               / static_cast<float>(h())
               ));
    }

    int w();
    int h();
private:
    bool fail_;
};

bool IsScreenValid();
Screen& GetScreen();
void SetScreen(Screen* scr);
