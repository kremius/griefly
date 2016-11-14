#pragma once

#include <QLineEdit>
#include <QWidget>
#include <QKeyEvent>

class GamingLineEdit: public QLineEdit
{
    Q_OBJECT
public:
    GamingLineEdit(QWidget* parent = 0);
    void keyPressEvent(QKeyEvent* event);
signals:
    void keyToPass(QKeyEvent* event);
private:
    bool is_arrow_pass_;
};
