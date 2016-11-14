#include "Item.h"

#include <sstream>

#include "../SyncRandom.h"
#include "Slot.h"

#include "../Game.h"

Item::Item(quint32 id) : IMovable(id)
{
    passable_level = Passable::SMALL_ITEM;

    type = Slots::DEFAULT;

    damage = 0;

    v_level = 5;
    SetSprite("icons/items.dmi");
}

Screwdriver::Screwdriver(quint32 id) : Item(id)
{
    damage = 5;
    SetState("screwdriver");
    name = "Screwdriver";
}

void Screwdriver::AfterWorldCreation()
{
    Item::AfterWorldCreation();

    unsigned int value = GetRand() % 8;
    SetState(QString("screwdriver%1").arg(value));
}

Wirecutters::Wirecutters(quint32 id) : Item(id)
{
    SetState("cutters");

    damage = 10;

    name = "Wirecutters";
}

Crowbar::Crowbar(quint32 id) : Item(id)
{
    SetState("crowbar");

    damage = 15;

    name = "Crowbar";
}

Wrench::Wrench(quint32 id) : Item(id)
{
    SetState("wrench");

    damage = 5;

    name = "Wrench";
}
BulletCasing::BulletCasing(quint32 id) : Item(id)
{
    SetSprite("icons/ammo.dmi");
    SetState("s-casing");

    damage = 0;

    name = "Bullet Casing";
}
