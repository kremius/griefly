#include "RemoteDoorOpener.h"

RemoteDoorOpener::RemoteDoorOpener(quint32 id)
    : Item(id)
{
    SetSprite("icons/device.dmi");
    SetState("multitool_red");
    name = "Atmos tool";

    door_ = 0;
}

void RemoteDoorOpener::AttackBy(IdPtr<Item> item)
{
    if (item.Id() == GetId())
    {
        if (door_.IsValid())
        {
            door_->Open();
        }
    }
}

void RemoteDoorOpener::SetDoor(IdPtr<Door> door)
{
    door_ = door;
}
