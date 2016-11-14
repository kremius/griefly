#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <functional>

#include "core/Constheader.h"

#include <QObject>
#include <QElapsedTimer>

struct NameSize
{
    QString name;
    unsigned int size;
};

class LessNameSize
{
public:
    bool operator() (const NameSize& left, const NameSize& right);
};

class TextPainter: public QObject
{
    Q_OBJECT
public:
    class Text
    {
        friend class TextPainter;
    public:
        Text& SetUpdater(std::function<void(QString*)> updater);
        Text& SetFreq(int freq);
    private:
        Text(TextPainter* master, const QString& key);
        ~Text();
        void Update();
        bool CanUpdate();

        unsigned int how_often_; // ms
        std::function<void(QString*)> updater_;

        QString key_;

        QString content_;
        QElapsedTimer time_;

        TextPainter* master_;
    };

    virtual ~TextPainter();

    void Process();
    bool Delete(const QString& name);
    Text& operator[](const QString& name);
    void AddSystemText(QString key, QString text);
signals:
    void addSystemText(QString key, QString text);
private:
    std::map<QString, Text*> texts_;
};
