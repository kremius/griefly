#ifndef QTOPENGL_H
#define QTOPENGL_H

#include <QGLWidget>

class QtOpenGL : public QGLWidget
{
    Q_OBJECT
public:
    explicit QtOpenGL(QWidget *parent = 0);
    ~QtOpenGL();

signals:
    void enterPressed();
    void f2Pressed();
    void focusLost();
public slots:
    void handlePassedKey(QKeyEvent* event);
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
};

QtOpenGL* GetGLWidget();
void MakeCurrentGLContext();
void SetGLContext(QtOpenGL* widget);

#endif // QTOPENGL_H
