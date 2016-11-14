#pragma once

#include <string>

#include "core/objects/Mob.h"
#include "Interfaces.h"

#include <QTextBrowser>
#include <QObject>

class Chat: public QObject, public IChat
{
    Q_OBJECT
public:
    static bool IsOOCMessage(const QString& text);
    Chat(IGame* game);

    void PostTextFor(const QString& str, IdPtr<IOnMapBase> owner);
    void PostText(const QString& str);
    void PostOOCText(const QString& who, const QString& str);
    void PostSimpleText(const QString& str, quint32 tile_id);
    void PostDamage(const QString& by, const QString& who, const QString& object, quint32 tile_id);
    void PostWords(const QString& who, const QString& text, quint32 tile_id);
signals:
    void insertHtmlIntoChat(QString html);
private:
    IGame* game_;
};
