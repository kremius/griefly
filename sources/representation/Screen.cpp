#include "Screen.h"

#include <cassert>

#include "Params.h"
#include "qt/qtopengl.h"

Screen::Screen(unsigned int x, unsigned int y, bool fullscreen)
{
    fail_ = true;
    ResetScreen(x, y);
    fail_ = false;
}

void Screen::PerformSizeUpdate()
{
    glViewport(0, 0, GetGLWidget()->width(), GetGLWidget()->height());
}

void Screen::ResetScreen(int x, int y)
{
    GetGLWidget()->resize(x, y);
}

void Screen::Swap()
{
    GetGLWidget()->swapBuffers();
}

void Screen::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void Screen::Draw(const GLSprite* sprite_in, int x, int y, int imageW, int imageH, float angle)
{
    if (sprite_in == nullptr)
    {
        return;
    }

    const GLSprite& sprite = *sprite_in;

    glColor4f(1, 1, 1, 1);
    glBindTexture(GL_TEXTURE_2D, sprite[imageH][imageW]);
    if (glGetError())
    {
        qDebug() << glGetError();
    }
    glTranslatef(     x + sprite.W() / 2.0f,      y + sprite.H() / 2.0f, 0.0f); 
    glRotatef(angle, 0.0f, 0.0f, 1.0f);
    glTranslatef(-1 * x - sprite.W() / 2.0f, -1 * y - sprite.H() / 2.0f, 0.0f); 

    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 1.0);     glVertex3f(static_cast<float>(x),              static_cast<float>(y), 0.0f);
        glTexCoord2f(0.0, 0.0);     glVertex3f(static_cast<float>(x),              static_cast<float>(y + sprite.H()), 0.0f);
        glTexCoord2f(1.0, 0.0);     glVertex3f(static_cast<float>(x + sprite.W()), static_cast<float>(y + sprite.H()), 0.0f);
        glTexCoord2f(1.0, 1.0);     glVertex3f(static_cast<float>(x + sprite.W()), static_cast<float>(y), 0.0f);
    glEnd();
    glLoadIdentity();

    if (glGetError())
    {
        qDebug() << glGetError();
    }

}

int Screen::w()
{
    return GetGLWidget()->width();
}
int Screen::h()
{
    return GetGLWidget()->height();
}

Screen* screen = nullptr;
void SetScreen(Screen* scr)
{
    screen = scr;
}
Screen& GetScreen()
{
    return *screen;
}


bool IsScreenValid()
{
    return screen != nullptr;
}
