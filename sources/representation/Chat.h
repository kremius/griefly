#pragma once

#include <string>

#include "core/objects/Mob.h"
#include "Interfaces.h"

#include <QTextBrowser>
#include <QObject>

class Chat : public ChatInterface
{
public:
    Chat(GameInterface* game);

    virtual void PostTextFor(const QString& str, IdPtr<kv::MapObject> owner) override;
    virtual void PostHtmlFor(const QString& str, IdPtr<kv::MapObject> owner) override;
    virtual void PostText(const QString& str) override;
    virtual void PostSimpleText(const QString& str, quint32 tile_id) override;
    virtual void PostHtmlText(const QString& str, quint32 tile_id) override;
    virtual void PostDamage(const QString& by, const QString& who, const QString& object, quint32 tile_id) override;
private:
    GameInterface* game_;
};
