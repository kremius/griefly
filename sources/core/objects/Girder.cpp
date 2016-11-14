#include "Girder.h"

#include "Materials.h"
#include "MetalWall.h"
#include "Item.h"

#include "../ObjectFactory.h"
#include "../Game.h"

#include "representation/Sound.h"

Girder::Girder(quint32 id) : Structure(id)
{
    SetSprite("icons/structures.dmi");
    SetState("girder");

    SetPassable(D_ALL, Passable::AIR);

    anchored = true;
    name = "Girder";
}

void Girder::AttackBy(IdPtr<Item> item)
{
    if (IdPtr<Metal> metal = item)
    {
        metal->Delete();

        GetTurf()->Delete();
        Create<ITurf>(MetalWall::T_ITEM_S(), GetOwner());

        Delete();
    }
    else if (IdPtr<Wrench> wrench = item)
    {
        PlaySoundIfVisible("Ratchet.ogg", owner.Id());
        Create<Item>(Metal::T_ITEM_S(), GetOwner());
        Delete();
    }
}
