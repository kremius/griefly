#include "Text.h"

#include "Screen.h"

TextPainter::Text::Text(TextPainter* master, const QString& key)
    : updater_([](QString*){}),
    master_(master),
    how_often_(200),
    key_(key)
{
    time_.start();
}

TextPainter::Text::~Text()
{
}

TextPainter::Text& TextPainter::Text::SetUpdater(std::function<void(QString*)> updater)
{
    updater_ = updater;
    return *this;
}

TextPainter::Text& TextPainter::Text::SetFreq(int freq)
{
    how_often_ = freq;
    return *this;
}

void TextPainter::Text::Update()
{
    QString new_content;
    updater_(&new_content);
    if (content_ == new_content)
    {
        return;
    }
    content_.clear();
    content_ = new_content;
    master_->AddSystemText(key_, content_);
}

bool TextPainter::Text::CanUpdate()
{
    if (time_.elapsed() > how_often_)
    {
        time_.restart();
        return true;
    }
    return false;
}

TextPainter::~TextPainter()
{

}

void TextPainter::Process()
{
    for (auto it = texts_.begin(); it != texts_.end(); ++it)
    {
        if (it->second->CanUpdate())
        {
            it->second->Update();
        }
    }
}

bool TextPainter::Delete(const QString& name)
{
    auto itr = texts_.find(name);
    if (itr == texts_.end())
    {
        return false;
    }
    
    emit addSystemText(name, "");

    delete itr->second;
    texts_.erase(itr);
    return true;
}

TextPainter::Text& TextPainter::operator[](const QString& name)
{
    auto value = texts_[name];
    if (value == nullptr)
    {
        texts_[name] = new Text(this, name);
    }
    return *texts_[name];
}

void TextPainter::AddSystemText(QString key, QString text)
{
    emit addSystemText(key, text);
}
