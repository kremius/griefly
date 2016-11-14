#include "Floor.h"

#include "representation/Sound.h"

#include "Item.h"
#include "FloorTile.h"
#include "Pipes.h"

#include "../ObjectFactory.h"
#include "../Game.h"

Floor::Floor(quint32 id) : ITurf(id)
{
    transparent = true;
    SetPassable(D_ALL, Passable::FULL);

    SetSprite("icons/floors.dmi");
    SetState("floor");

    name = "Floor";

    open_ = false;
    bloody = false;
}

void Floor::AfterWorldCreation()
{
    ITurf::AfterWorldCreation();
    SetOpen(open_);
}

void Floor::AttackBy(IdPtr<Item> item)
{
    if (IdPtr<Crowbar> c = item)
    {
        if (!open_)
        {
            SetOpen(true);
            Create<Item>(FloorTile::T_ITEM_S(), GetOwner());
            PlaySoundIfVisible("Crowbar.ogg", owner.Id());
        }
    }
    else if (IdPtr<FloorTile> ftile = item)
    {
        if (open_)
        {
            SetOpen(false);
            ftile->Delete();
            PlaySoundIfVisible("Deconstruct.ogg", owner.Id());
        }
    }
}

void Floor::SetOpen(bool o)
{
    //qDebug() << "Start set open " << owner->GetId();
    open_ = o;
    if (open_)
    {
        SetState("plating");
        v_level = 0;
        if (auto vent = owner->GetItem<Vent>())
        {
            vent->SetHidden(false);
        }
    }
    else
    {
        SetState("floor");
        v_level = 2;
        if (auto vent = owner->GetItem<Vent>())
        {
            vent->SetHidden(true);
        }
    }
    GetView()->RemoveOverlays();
    //qDebug() << "End setopen";
}


Plating::Plating(quint32 id) : Floor(id)
{
    open_ = true;
    // For map editor
    SetState("plating");
}
