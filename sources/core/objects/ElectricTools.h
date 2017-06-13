#pragma once

#include "Item.h"

#include "../atmos/AtmosHolder.h"

namespace kv
{

class AtmosTool : public Item
{
public:
    DECLARE_SAVEABLE(AtmosTool, Item);
    REGISTER_CLASS_AS(AtmosTool);
    AtmosTool();

    static QString GetHtmlInfo(const atmos::AtmosHolder& holder);

    virtual void AttackBy(IdPtr<Item> item) override;
};
END_DECLARE(AtmosTool)

class RemoteAtmosTool : public AtmosTool
{
public:
    DECLARE_SAVEABLE(RemoteAtmosTool, AtmosTool);
    REGISTER_CLASS_AS(RemoteAtmosTool);
    RemoteAtmosTool();
};
END_DECLARE(RemoteAtmosTool)

}
