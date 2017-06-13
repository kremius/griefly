#include "Chat.h"

#include "Text.h"
#include "core/Constheader.h"
#include "Screen.h"
#include "core/Helpers.h"
#include "core/Game.h"

#include <QUuid>
#include <QTextCursor>

Chat::Chat(GameInterface* game)
{
    game_ = game;
}

void Chat::PostText(const QString& str)
{
    QString loc = str.toHtmlEscaped();
    loc.replace('\n', "<br>");
    GetRepresentation().AddToNewFrame(Representation::ChatMessage{loc});
}
