#pragma once

#include "Item.h"

#include "../atmos/AtmosHolder.h"

class AtmosTool: public Item
{
public:
    DECLARE_SAVED(AtmosTool, Item);
    DECLARE_GET_TYPE_ITEM(AtmosTool);
    AtmosTool(quint32 id);

    static QString GetInfo(AtmosHolder& holder);

    virtual void AttackBy(IdPtr<Item> item) override;
};
ADD_TO_TYPELIST(AtmosTool)

class RemoteAtmosTool: public AtmosTool
{
public:
    DECLARE_SAVED(RemoteAtmosTool, AtmosTool);
    DECLARE_GET_TYPE_ITEM(RemoteAtmosTool);
    RemoteAtmosTool(quint32 id);
};
ADD_TO_TYPELIST(RemoteAtmosTool)
