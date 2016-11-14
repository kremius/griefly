#pragma once

#include "core/objects/Item.h"
#include "core/objects/Door.h"

class RemoteDoorOpener : public Item
{
public:
    DECLARE_SAVED(RemoteDoorOpener, Item);
    DECLARE_GET_TYPE_ITEM(RemoteDoorOpener);
    RemoteDoorOpener(quint32 id);

    virtual void AttackBy(IdPtr<Item> item) override;

    void SetDoor(IdPtr<Door> door);
private:
    IdPtr<Door> KV_SAVEBLE(door_);
};
ADD_TO_TYPELIST(RemoteDoorOpener)
