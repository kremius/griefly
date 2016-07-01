#include "Item.h"

#include <sstream>

#include "../SyncRandom.h"
#include "Slot.h"

#include "../Game.h"

Item::Item(size_t id) : IMovable(id)
{
    passable_level = Passable::SMALL_ITEM;

    type = Slots::DEFAULT;

    damage = 0;

    v_level = 5;
    SetSprite("icons/items.dmi");
}

Screwdriver::Screwdriver(size_t id) : Item(id)
{
    damage = 5;
    SetState("screwdriver");
    name = "Screwdriver";
}

void Screwdriver::AfterWorldCreation()
{
    Item::AfterWorldCreation();

    unsigned int value = GetRand() % 8;
    std::stringstream converter;
    converter << "screwdriver";
    if (value > 1)
    {
        converter << value;
    }
    SetState(converter.str());
}

Wirecutters::Wirecutters(size_t id) : Item(id)
{
    SetState("cutters");

    damage = 10;

    name = "Wirecutters";
}

Crowbar::Crowbar(size_t id) : Item(id)
{
    SetState("crowbar");

    damage = 15;

    name = "Crowbar";
}

Wrench::Wrench(size_t id) : Item(id)
{
    SetState("wrench");

    damage = 5;

    name = "Wrench";
}
BulletCasing::BulletCasing(size_t id) : Item(id)
{
    SetSprite("icons/ammo.dmi");
    SetState("s-casing");

    damage = 0;

    name = "Bullet Casing";
}
