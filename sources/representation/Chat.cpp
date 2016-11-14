#include "Chat.h"

#include <sstream>

#include "Text.h"
#include "core/Constheader.h"
#include "Screen.h"
#include "core/Helpers.h"
#include "core/Game.h"

#include <QUuid>
#include <QTextCursor>

Chat::Chat(IGame* game)
{
    game_ = game;
}

void Chat::PostSimpleText(const QString& str, quint32 tile_id)
{
    if (!game_->GetMap().IsTileVisible(tile_id))
    {
        return;
    }

    emit insertHtmlIntoChat(str);
}

void Chat::PostDamage(const QString& by, const QString& who, const QString& object, quint32 tile_id)
{
    if (!game_->GetMap().IsTileVisible(tile_id))
    {
        return;
    }

    QString q_by = by.toHtmlEscaped();
    QString q_who = who.toHtmlEscaped();
    QString q_object = object.toHtmlEscaped();

    emit insertHtmlIntoChat
        (
           "<font color=\"red\">"
         + q_who
         + " is attacked by "
         + q_by
         + " with "
         + q_object
         + "</font>");
}

void Chat::PostWords(const QString& who, const QString& text, quint32 tile_id)
{
    if (!game_->GetMap().IsTileVisible(tile_id))
    {
        return;
    }

    QString q_who = who.toHtmlEscaped();
    QString q_text = text.toHtmlEscaped();

    emit insertHtmlIntoChat
        (
           "<b>"
         + q_who
         + "</b>: "
         + "<span>"
         + q_text
         + "</span>");
}

void Chat::PostTextFor(const QString& str, IdPtr<IOnMapBase> owner)
{
    if (game_->GetMob() == owner)
    {
        PostText(str);
    }
}

void Chat::PostText(const QString& str_)
{
    QString loc = str_.toHtmlEscaped();
    emit insertHtmlIntoChat(loc.replace('\n', "<br>"));
}

void Chat::PostOOCText(const QString &who, const QString& text)
{
    QString q_who = who.toHtmlEscaped();
    QString q_text = text.toHtmlEscaped();

    emit insertHtmlIntoChat
        (
           "<font color=\"blue\"><b>"
         + q_who
         + "</b>: "
         + "<span>"
         + q_text
         + "</span></font>");
}
