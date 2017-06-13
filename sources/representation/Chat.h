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

    virtual void PostText(const QString& str) override;
private:
    GameInterface* game_;
};
